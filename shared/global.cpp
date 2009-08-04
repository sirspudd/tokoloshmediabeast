#include "global.h"

void initApp(const QString &appname, int argc, char **argv)
{
    QCoreApplication::setApplicationName(appname);
    QCoreApplication::setOrganizationName("Donders");
    qDBusRegisterMetaType<TrackData>();
    qDBusRegisterMetaType<QHash<int, int> >();
    qDBusRegisterMetaType<Function>();

    Config::init(argc, argv);
}

TrackData &TrackData::operator|=(const TrackData &other)
{
    for (int i=0; trackInfos[i] != None; ++i) {
        const TrackInfo info = trackInfos[i];
        if (other.fields & info && !(fields & info)) {
            setData(info, other.data(info));
        }
    }
    return *this;
}

QVariant TrackData::data(TrackInfo info) const
{
    if (!(info & fields)) {
        return QVariant();
    }
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
    case URL: fields |= (FileName|URL); url = data.toUrl(); break;
    case Title: title = data.toString(); break;
    case TrackLength: trackLength = data.toInt(); break;
    case Artist: artist = data.toString(); break;
    case Album: album = data.toString(); break;
    case Year: year = data.toInt(); break;
    case Genre: genre = data.toString(); break;
    case AlbumIndex: albumIndex = data.toInt(); break;
    case PlaylistIndex: playlistIndex = data.toInt(); break;
    case FileName: Q_ASSERT(0); break; //
    case None:
    case All:
        Q_ASSERT(0);
        break;
    }
    fields |= info;
}

QString TrackData::toString() const
{
    const TrackInfo *info = ::trackInfos;
    QString ret;
    ret.reserve(256);
    while (*info != None) {
        const QVariant value = data(*info);
        if (!value.isNull()) {
            QString stringValue;
            if (*info == URL) {
                stringValue = url.toString();
            } else {
                stringValue = value.toString();
            }
            Q_ASSERT(!stringValue.isEmpty());
            ret.append(::trackInfoToString(*info));
            ret.append(": ");
            ret.append(stringValue);
            ret.append('\n');
        }
        ++info;
    }
    return ret;
}


QDBusArgument &operator<<(QDBusArgument &arg, const TrackData &trackData)
{
    arg.beginStructure();
    // enum { Version = 1 }; ### Version stuff?
    QByteArray data;
    {
        QDataStream ds(&data, QIODevice::WriteOnly);
        ds << qint32(trackData.fields);
        if (trackData.fields & URL)
            ds << trackData.url;
        if (trackData.fields & Title)
            ds << trackData.title;
        if (trackData.fields & Artist)
            ds << trackData.artist;
        if (trackData.fields & Album)
            ds << trackData.album;
        if (trackData.fields & Genre)
            ds << trackData.genre;
        if (trackData.fields & TrackLength)
            ds << trackData.trackLength;
        if (trackData.fields & AlbumIndex)
            ds << trackData.albumIndex;
        if (trackData.fields & Year)
            ds << trackData.year;
        if (trackData.fields & PlaylistIndex)
            ds << trackData.playlistIndex;
    }
    arg << data;
    arg.endStructure();
    return arg;
}


const QDBusArgument &operator>>(const QDBusArgument &arg, TrackData &trackData)
{
    arg.beginStructure();
    QByteArray data;
    arg >> data;
    QDataStream ds(&data, QIODevice::ReadOnly);
    // enum { Version = 1 }; ### Version stuff?
    qint32 tmp;
    ds >> tmp;
    trackData.fields = tmp;
    if (trackData.fields & URL)
        ds >> trackData.url;
    if (trackData.fields & Title)
        ds >> trackData.title;
    if (trackData.fields & Artist)
        ds >> trackData.artist;
    if (trackData.fields & Album)
        ds >> trackData.album;
    if (trackData.fields & Genre)
        ds >> trackData.genre;
    if (trackData.fields & TrackLength)
        ds >> trackData.trackLength;
    if (trackData.fields & AlbumIndex)
        ds >> trackData.albumIndex;
    if (trackData.fields & Year)
        ds >> trackData.year;
    if (trackData.fields & PlaylistIndex)
        ds >> trackData.playlistIndex;
    arg.endStructure();
    return arg;
}


QDBusArgument &operator<<(QDBusArgument &arg, const QHash<int, int> &hash)
{
    arg.beginStructure();
    arg << qint32(hash.size());
    for (QHash<int, int>::const_iterator it = hash.begin(); it != hash.end(); ++it) {
        arg << qint32(it.key()) << qint32(it.value());
    }
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, QHash<int, int> &hash)
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
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const Function &func)
{
    arg.beginStructure();
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << func.name << func.args << func.returnArgument;
    arg << ba;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, Function &func)
{
    QByteArray ba;
    arg.beginStructure();
    arg >> ba;
    QDataStream ds(ba);
    ds >> func.name >> func.args >> func.returnArgument;
    arg.endStructure();
    return arg;
}
