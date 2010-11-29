Copyright (c) 2010 Anders Bakken
Copyright (c) 2010 Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore>
#include <QtDBus>
#include <config.h>

#define SERVICE_NAME  "com.TokoloshMediaPlayer"

enum TrackInfo {
    None = 0x000,
    URL = 0x001,
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

static inline QString trackInfoToString(TrackInfo info)
{
    switch (info) {
    case URL: return QCoreApplication::translate("TrackInfo", "URL");
    case Title: return QCoreApplication::translate("TrackInfo", "Title");
    case TrackLength: return QCoreApplication::translate("TrackInfo", "TrackLength");
    case Artist: return QCoreApplication::translate("TrackInfo", "Artist");
    case Album: return QCoreApplication::translate("TrackInfo", "Album");
    case Year: return QCoreApplication::translate("TrackInfo", "Year");
    case Genre: return QCoreApplication::translate("TrackInfo", "Genre");
    case AlbumIndex: return QCoreApplication::translate("TrackInfo", "AlbumIndex");
    case PlaylistIndex: return QCoreApplication::translate("TrackInfo", "PlaylistIndex");
    case FileName: return QCoreApplication::translate("TrackInfo", "FileName");
    default:
        break;
    }
    Q_ASSERT(0);
    return QString();
}

static const TrackInfo trackInfos[] = { URL, Title, TrackLength, Artist, Album, Year, Genre, AlbumIndex, PlaylistIndex, None };
static inline QStringList trackInfosToStringList(uint info)
{
    QStringList ret;
    for (int i=0; trackInfos[i] != None; ++i) {
        if (info & trackInfos[i]) {
            ret.append(trackInfoToString(trackInfos[i]));
        }
    }
    if (info & FileName)
        ret.append(trackInfoToString(FileName));
    return ret;
}

void initApp(const QString &appname, int argc, char **argv);
struct TrackData
{
    TrackData() : trackLength(-1), albumIndex(-1), year(-1), playlistIndex(-1), fields(None) {}

    QUrl url;
    QString title, artist, album, genre;
    int trackLength, albumIndex, year, playlistIndex; // seconds

    int fields; // this means which fields have been queried. values
    // might still be null if we don't know about them
    // but there's no reason to ask again.

    QVariant data(TrackInfo info) const;
    void setData(TrackInfo info, const QVariant &data);
    QString toString() const;
    TrackData &operator|=(const TrackData &other);
};

Q_DECLARE_METATYPE(TrackData);
QDBusArgument &operator<<(QDBusArgument &arg, const TrackData &trackData);
const QDBusArgument &operator>>(const QDBusArgument &arg, TrackData &trackData);
typedef QHash<int, int> IntHash;
Q_DECLARE_METATYPE(IntHash);
QDBusArgument &operator<<(QDBusArgument &arg, const QHash<int, int> &ih);
const QDBusArgument &operator>>(const QDBusArgument &arg, QHash<int, int> &ih);

/* There could be overloads of the function. The different types of arguments are in args */
struct Function
{
    Function() : returnArgument(0) {}
    QString name;
    QList<QList<int> > args;
    int returnArgument;
};

Q_DECLARE_METATYPE(Function);
QDBusArgument &operator<<(QDBusArgument &arg, const Function &f);
const QDBusArgument &operator>>(const QDBusArgument &arg, Function &f);

Q_DECLARE_METATYPE(QUrl);
static inline QDBusArgument &operator<<(QDBusArgument &arg, const QUrl &url)
{
    arg << url.toString();
    return arg;
}

static inline const QDBusArgument &operator>>(const QDBusArgument &arg, QUrl &url)
{
    QString str;
    arg >> str;
    url = QUrl(str);
    return arg;
}

static inline bool operator==(const Function &left, const Function &right)
{ return left.name == right.name && left.args == right.args; }

static inline bool operator!=(const Function &left, const Function &right)
{ return left.name != right.name || left.args != right.args; }

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
