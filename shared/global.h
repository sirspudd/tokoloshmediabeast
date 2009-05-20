#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore>

enum TrackInfo {
    None = 0x000,
    FilePath = 0x001,
    Title = 0x002,
    TrackLength = 0x004,
    Artist = 0x008,
    Album = 0x010,
    Year = 0x020,
    Genre = 0x040,
    TrackNumber = 0x080,
    PlaylistIndex = 0x100,
    All = 0xfff
};

static inline void initApp(QCoreApplication *app, const QString &appname)
{
    app->setApplicationName(appname);
    app->setOrganizationName(QLatin1String("Donders"));
    app->setOrganizationDomain(QLatin1String("http://github.com/sirspudd/tokoloshmediabeast/tree/master"));
}

struct TrackData
{
    TrackData() : trackLength(-1), trackNumber(-1), year(-1), playlistIndex(-1) {}

    QString path, title, artist, album, genre;
    int trackLength, trackNumber, year, playlistIndex; // seconds
};

Q_DECLARE_METATYPE(TrackData);

static inline QDataStream &operator<<(QDataStream &ds, const TrackData &data)
{
    enum { Version = 1 };
    ds << quint8(Version) << data.path << data.trackNumber
       << data.artist << data.album << data.genre
       << data.trackLength << data.trackNumber << data.year << data.playlistIndex;
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
    ds >> data.path >> data.trackNumber
       >> data.artist >> data.album >> data.genre
       >> data.trackLength >> data.trackNumber
       >> data.year >> data.playlistIndex;
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



#endif
