#include "global.h"

void initApp(const QString &appname, int argc, char **argv)
{
    QCoreApplication::setApplicationName(appname);
    QCoreApplication::setOrganizationName("Donders");
    qDBusRegisterMetaType<TrackData>();
    qDBusRegisterMetaType<QHash<int, int> >();
    qDBusRegisterMetaType<Function>();
    qDBusRegisterMetaType<QList<Function> >();

    Config::init(argc, argv);
}

TrackData &TrackData::operator|=(const TrackData &other)
{
    for (int i=0; trackInfos[i] != None; ++i) {
        const TrackInfo info = trackInfos[i];
        if (other.fields & info) {
            setData(info, other.data(info));
        }
    }
    return *this;
}

QVariant TrackData::data(TrackInfo info) const
{
    if (!(info & fields))
        return QVariant();
    switch (info) {
    case URL: return url;
    case Title: return title;
    case TrackLength: return trackLength;
    case Artist: return artist;
    case Album: return album;
    case Year: return year;
    case Genre: return genre;
    case AlbumIndex: return albumIndex;
    case PlaylistIndex: return playlistIndex;
    case FileName: return QFileInfo(url.path()).fileName();
    case None:
    case All: break;
    }
    Q_ASSERT(0);
    return QVariant();
}

void TrackData::setData(TrackInfo info, const QVariant &data)
{
    switch (info) {
    case URL: fields |= FileName; url = data.toString(); break;
    case Title: title = data.toString(); break;
    case TrackLength: trackLength = data.toInt(); break;
    case Artist: artist = data.toInt(); break;
    case Album: album = data.toString(); break;
    case Year: year = data.toInt(); break;
    case Genre: genre = data.toString(); break;
    case AlbumIndex: albumIndex = data.toInt(); break;
    case PlaylistIndex: playlistIndex = data.toInt(); break;
    case FileName: break; //
    case None:
    case All:
        Q_ASSERT(0);
        break;
    }
    fields |= info;
}

void operator<<(QDBusArgument &arg, const TrackData &trackData)
{
    arg.beginStructure();
    // enum { Version = 1 }; ### Version stuff?
    // ### could optimize both this one and QDataStream one to only read the fields in fields
    arg << qint32(trackData.fields);
    if (trackData.fields & URL)
        arg << trackData.url;
    if (trackData.fields & Title)
        arg << trackData.title;
    if (trackData.fields & Artist)
        arg << trackData.artist;
    if (trackData.fields & Album)
        arg << trackData.album;
    if (trackData.fields & Genre)
        arg << trackData.genre;
    if (trackData.fields & TrackLength)
        arg << trackData.trackLength;
    if (trackData.fields & AlbumIndex)
        arg << trackData.albumIndex;
    if (trackData.fields & Year)
        arg << trackData.year;
    if (trackData.fields & PlaylistIndex)
        arg << trackData.playlistIndex;
//     arg << qint32(trackData.fields) << trackData.url
//         << trackData.title << trackData.artist << trackData.album
//         << trackData.genre << trackData.trackLength
//         << trackData.albumIndex << trackData.year
//         << trackData.playlistIndex;
    arg.endStructure();
}


void operator>>(const QDBusArgument &arg, TrackData &trackData)
{
    arg.beginStructure();
    // enum { Version = 1 }; ### Version stuff?
    // ### could optimize both this one and QDataStream one to only read the fields in fields
    arg >> *reinterpret_cast<qint32*>(&trackData.fields);
    if (trackData.fields & URL)
        arg >> trackData.url;
    if (trackData.fields & Title)
        arg >> trackData.title;
    if (trackData.fields & Artist)
        arg >> trackData.artist;
    if (trackData.fields & Album)
        arg >> trackData.album;
    if (trackData.fields & Genre)
        arg >> trackData.genre;
    if (trackData.fields & TrackLength)
        arg >> trackData.trackLength;
    if (trackData.fields & AlbumIndex)
        arg >> trackData.albumIndex;
    if (trackData.fields & Year)
        arg >> trackData.year;
    if (trackData.fields & PlaylistIndex)
        arg >> trackData.playlistIndex;
    arg.endStructure();
}


void operator<<(QDBusArgument &arg, const QHash<int, int> &hash)
{
    arg.beginStructure();
    arg << qint32(hash.size());
    for (QHash<int, int>::const_iterator it = hash.begin(); it != hash.end(); ++it) {
        arg << qint32(it.key()) << qint32(it.value());
    }
    arg.endStructure();
}

void operator>>(const QDBusArgument &arg, QHash<int, int> &hash)
{
    arg.beginStructure();
    hash.clear();
    qint32 count;
    arg >> count;
    for (int i=0; i<count; ++i) {
        qint32 key, val;
        arg >> key >> val;
        hash[key] = val;
    }
    arg.endStructure();
}

void operator<<(QDBusArgument &arg, const Function &func)
{
    arg.beginStructure();
    arg << func.name << func.args.size();
    foreach(int type, func.args)
        arg << type;
    arg.endStructure();
}

void operator>>(const QDBusArgument &arg, Function &func)
{
    arg.beginStructure();
    arg >> func.name;
    int count;
    arg >> count;
    func.args.clear();
    for (int i=0; i<count; ++i) {
        int tmp;
        arg >> tmp;
        func.args.append(tmp);
    }
    arg.endStructure();
}
