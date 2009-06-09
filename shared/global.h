#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore>

#define SERVICE_NAME  "com.TokoloshMediaPlayer"

enum TrackInfo {
    None = 0x000,
    FilePath = 0x001,
    Title = 0x002,
    TrackLength = 0x004,
    Artist = 0x008,
    Album = 0x010,
    Year = 0x020,
    Genre = 0x040,
    AlbumIndex = 0x080,
    PlaylistIndex = 0x100,
    FileName = 0x200,
    All = 0xfff
};

static const TrackInfo trackInfos[] = { FilePath, Title, TrackLength, Artist, Album, Year, Genre, AlbumIndex, PlaylistIndex, None };
static inline void initApp(QCoreApplication *app, const QString &appname)
{
    app->setApplicationName(appname);
    app->setOrganizationName(QLatin1String("Donders"));
//     app->setOrganizationDomain(QLatin1String("www.github.com/sirspudd/tokoloshmediabeast/tree/master"));
//    qRegisterMetaType<QVariant>("QVariant");
}

struct TrackData
{
    TrackData() : trackLength(-1), albumIndex(-1), year(-1), playlistIndex(-1), fields(None) {}

    QString path, title, artist, album, genre;
    int trackLength, albumIndex, year, playlistIndex; // seconds

    int fields; // this means which fields have been queried. values
                 // might still be null if we don't know about them
                 // but there's no reason to ask again.

    inline QVariant data(TrackInfo info) const;
    inline void setData(TrackInfo info, const QVariant &data);
    TrackData &operator|=(const TrackData &other)
    {
        for (int i=0; trackInfos[i] != None; ++i) {
            const TrackInfo info = trackInfos[i];
            if (other.fields & info) {
                setData(info, other.data(info));
            }
        }
        return *this;
    }
};

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

Q_DECLARE_METATYPE(TrackData);

static inline QDataStream &operator<<(QDataStream &ds, const TrackData &data)
{
    enum { Version = 1 };
//    qDebug() << "streaming out" << data.path << data.fields;
    ds << quint8(Version) << qint32(data.fields) << data.path << data.albumIndex
       << data.artist << data.album << data.genre
       << data.trackLength << data.albumIndex << data.year << data.playlistIndex;
    return ds;
}

static inline QDataStream &operator>>(QDataStream &ds, TrackData &data)
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
    ds >> data.path >> data.albumIndex
       >> data.artist >> data.album >> data.genre
       >> data.trackLength >> data.albumIndex
       >> data.year >> data.playlistIndex;
//    qDebug() << "streaming in" << data.path << data.fields;

    return ds;
}

static inline QByteArray fromEq(const QHash<int, int> &eq)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << eq;
    return ba;
}

static inline QHash<int, int> toEq(const QByteArray &ba)
{
    QDataStream ds(ba);
    QHash<int, int> ret;
    ds >> ret;
    return ret;
}


#ifdef Q_WS_WIN
#include <windows.h>
#else
#include <unistd.h>
#endif
static inline void sleep(int msec)
{
#ifdef Q_WS_WIN
    Sleep(msec);
#else
    usleep(1000*msec);
#endif
}

#endif
