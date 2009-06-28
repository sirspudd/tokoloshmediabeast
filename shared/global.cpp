#include "global.h"

void initApp(const QString &appname, int argc, char **argv)
{
    QCoreApplication::setApplicationName(appname);
    QCoreApplication::setOrganizationName(QLatin1String("Donders"));
    qDBusRegisterMetaType<TrackData>();
    qDBusRegisterMetaType<IntHash>();
    Config::init(argc, argv);
//     app->setOrganizationDomain(QLatin1String("www.github.com/sirspudd/tokoloshmediabeast/tree/master"));
//    qRegisterMetaType<QVariant>("QVariant");
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
    case FilePath: return path;
    case Title: return title;
    case TrackLength: return trackLength;
    case Artist: return artist;
    case Album: return album;
    case Year: return year;
    case Genre: return genre;
    case AlbumIndex: return albumIndex;
    case PlaylistIndex: return playlistIndex;
    case FileName: return QFileInfo(path).fileName();
    case None:
    case All: break;
    }
    Q_ASSERT(0);
    return QVariant();
}

void TrackData::setData(TrackInfo info, const QVariant &data)
{
    switch (info) {
    case FilePath: fields |= FileName; path = data.toString(); break;
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

QDataStream &operator<<(QDataStream &ds, const TrackData &data)
{
    enum { Version = 1 };
//    qDebug() << "streaming out" << data.path << data.fields;
    ds << quint8(Version) << qint32(data.fields)
       << data.path << data.artist << data.album << data.genre
       << data.trackLength << data.albumIndex << data.year << data.playlistIndex;
    return ds;
}

QDataStream &operator>>(QDataStream &ds, TrackData &data)
{
    enum { Version = 1 };
    quint8 version;
    ds >> version;
    if (version != Version) {
        qWarning("Unexpected version, got %d, expected %d", version, Version);
        return ds;
    }
    qint32 fields;
    ds >> fields;
    data.fields = fields;
    ds >> data.path >> data.artist >> data.album
       >> data.genre >> data.trackLength
       >> data.albumIndex >> data.year
       >> data.playlistIndex;
//    qDebug() << "streaming in" << data.path << data.fields;

    return ds;
}

void operator<<(QDBusArgument &arg, const TrackData &trackData)
{
    arg.beginStructure();
    // enum { Version = 1 }; ### Version stuff?
    // ### could optimize both this one and QDataStream one to only read the fields in fields
    arg << qint32(trackData.fields) << trackData.path
        << trackData.artist << trackData.album
        << trackData.genre << trackData.trackLength
        << trackData.albumIndex << trackData.year
        << trackData.playlistIndex;
    arg.endStructure();
//    qDebug() << "doing my thing reading" << foo.value;
}


void operator>>(const QDBusArgument &arg, TrackData &trackData)
{
    arg.beginStructure();
    // enum { Version = 1 }; ### Version stuff?
    // ### could optimize both this one and QDataStream one to only read the fields in fields
    arg >> *reinterpret_cast<qint32*>(&trackData.fields) >> trackData.path
        >> trackData.artist >> trackData.album
        >> trackData.genre >> trackData.trackLength
        >> trackData.albumIndex >> trackData.year
        >> trackData.playlistIndex;
    arg.endStructure();
//    qDebug() << "doing my thing reading" << foo.value;
}


void operator<<(QDBusArgument &arg, const IntHash &hash)
{
    arg.beginStructure();
    arg << qint32(hash.size());
    for (QHash<int, int>::const_iterator it = hash.begin(); it != hash.end(); ++it) {
        arg << qint32(it.key()) << qint32(it.value());
    }
    arg.endStructure();
}

void operator>>(const QDBusArgument &arg, IntHash &hash)
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
