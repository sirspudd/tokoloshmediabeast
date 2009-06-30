#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore>
#include <QtDBus>
#include <config.h>

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
void initApp(const QString &appname, int argc, char **argv);
struct TrackData
{
    TrackData() : trackLength(-1), albumIndex(-1), year(-1), playlistIndex(-1), fields(None) {}

    QString path, title, artist, album, genre;
    int trackLength, albumIndex, year, playlistIndex; // seconds

    int fields; // this means which fields have been queried. values
                 // might still be null if we don't know about them
                 // but there's no reason to ask again.

    QVariant data(TrackInfo info) const;
    void setData(TrackInfo info, const QVariant &data);
    TrackData &operator|=(const TrackData &other);
};
Q_DECLARE_METATYPE(TrackData);
void operator<<(QDBusArgument &arg, const TrackData &trackData);
void operator>>(const QDBusArgument &arg, TrackData &trackData);
typedef QHash<int, int> IntHash;
Q_DECLARE_METATYPE(IntHash);
void operator<<(QDBusArgument &arg, const QHash<int, int> &ih);
void operator>>(const QDBusArgument &arg, QHash<int, int> &ih);

struct Function
{
    QString name;
    QList<int> args;
};

Q_DECLARE_METATYPE(Function);
Q_DECLARE_METATYPE(QList<Function>);
void operator<<(QDBusArgument &arg, const Function &f);
void operator>>(const QDBusArgument &arg, Function &f);

static inline void operator<<(QDBusArgument &arg, const QList<Function> &f)
{
    arg << f;
}

static inline void operator>>(const QDBusArgument &arg, QList<Function> &f)
{
    f.clear();
    arg >> f;
}


static inline bool operator==(const Function &left, const Function &right)
{ return left.name == right.name && left.args == right.args; }

static inline bool operator!=(const Function &left, const Function &right)
{ return left.name != right.name || left.args != right.args; }

template <typename T>
static inline T readDBusMessage(const QDBusMessage &msg)
{
    if (msg.arguments().isEmpty() || msg.arguments().at(0).isNull()) {
        qWarning("Can't read this stuff");
        return T();
    }
    QDBusArgument arg = qVariantValue<QDBusArgument>(msg.arguments().value(0));
    T t;
    arg >> t;
    return t;
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
