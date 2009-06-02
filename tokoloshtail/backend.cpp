#include "backend.h"
#include <config.h>

Backend *Backend::inst = 0;
Backend *Backend::instance()
{
    return inst;
}

Backend::Backend()
{
    Q_ASSERT(!inst);
    inst = this;
}

void Backend::prev()
{
    if(!playlistData.tracks.size()) {
        return;
    }
    setCurrentTrack((playlistData.current +
                     playlistData.tracks.size() - 1) %
                        playlistData.tracks.size());
    play();
}

void Backend::next()
{
    if(!playlistData.tracks.size()) {
        return;
    }
    setCurrentTrack((playlistData.current + 1) %
                    playlistData.tracks.size());
    play();
}

int Backend::count() const
{
    return playlistData.tracks.size();
}

QString Backend::currentTrackName() const
{
    return playlistData.tracks.value(playlistData.current);
}

int Backend::currentTrackIndex() const
{
    return playlistData.current;
}

static inline bool tracks(const QStringList &list, int from, int size, QStringList *out)
{
    const int count = list.size();
    if (count != 0 && size != 0 && from >= 0 && from < count && (size < 0 || from + size <= count)) {
        if (size < 0)
            size = count - from;
        *out = list.mid(from, size);
        return true;
    }
    return false;
}


QStringList Backend::tracks(int start, int count) const
{
    QStringList ret;
    ::tracks(playlistData.tracks, start, count, &ret);
    return ret;
}

bool Backend::setCurrentTrack(int index)
{
    if (index >= 0 && index < playlistData.tracks.size()) {
        playlistData.current = index;
        const QString& filePath = playlistData.tracks.at(index);
        loadFile(filePath);
        emit currentTrackChanged(index, filePath); //, trackData(playlistData.tracks.at(index)));
        return true;
    } else {
        return false;
    }
}

bool Backend::setCurrentTrack(const QString &name)
{
    const int idx = playlistData.tracks.indexOf(name);
    if (idx != -1) {
        setCurrentTrack(idx);
        return true;
    }
    return false;
}

int Backend::indexOfTrack(const QString &name) const
{
    return playlistData.tracks.indexOf(name);
}

bool Backend::requestTrackData(const QString &filepath, uint trackInfo)
{
    TrackData data;
    if (trackInfo & FilePath)
        data.path = filepath;
    if (trackInfo & PlaylistIndex)
        data.playlistIndex = indexOfTrack(filepath);
    enum { BackendTypes = Title|TrackLength|Artist|Year|Genre|TrackNumber };
    if (trackInfo & BackendTypes) {
        trackData(&data, filepath, trackInfo); // ### check return value?
    }
    emit trackData(filepath, qVariantFromValue(data));
    // ### should check if the file exists in list??
    return indexOfTrack(filepath) != -1;
}

bool Backend::requestTracknames(int start, int count)
{
    QStringList ret;
    if (::tracks(playlistData.tracks, start, count, &ret)) {
        emit trackNames(start, ret);
        return true;
    }
    return false;
}

static inline uint qHash(const QFileInfo &fi) { return qHash(fi.absoluteFilePath()); }
static inline QStringList recursiveLoad(const QFileInfo &file, bool recurse, QSet<QFileInfo> *seen)
{
    QStringList ret;
    if (seen->contains(file)) {
        qWarning("Recursive symlink detected %s", qPrintable(file.absoluteFilePath()));
        return ret;
    }
    seen->insert(file);
    if (file.isSymLink()) {
        static const bool resolveSymlinks = Config::isEnabled("resolvesymlinks", true);
        if (resolveSymlinks)
            ret += ::recursiveLoad(file.readLink(), recurse, seen);
    } else if (file.isDir()) {
        const QDir dir(file.absoluteFilePath());
        const QFileInfoList list = dir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot|(recurse
                                                                                       ? QDir::Dirs
                                                                                       : static_cast<QDir::Filter>(0)));
        foreach(const QFileInfo &f, list) {
            Q_ASSERT(!f.isDir() || recurse);
            ret += ::recursiveLoad(f, recurse, seen);
        }
    } else if (file.isFile()) {
        const QString path = file.absoluteFilePath();
        static QPointer<Backend> backend = Backend::instance(); // could just be a member function
        if (backend->isValid(path) || true) {
            ret += path;
            // ### should I load song name here? Probably. Checking if
            // ### it's valid probably means parsing header anyway
        }
    }
    return ret;
}

bool Backend::load(const QString &path, bool recurse)
{
    const QFileInfo fi(path);
    qDebug() << fi.absoluteFilePath() << recurse << fi.isDir();
    if (!fi.exists()) {
        qWarning("%s doesn't seem to exist", qPrintable(path));
        return false;
    }
    QSet<QFileInfo> seen;
    const QStringList songs = ::recursiveLoad(path, recurse, &seen);
    if (!songs.isEmpty()) {
        playlistData.tracks.append(songs);
        emit trackCountChanged(playlistData.tracks.size());
        if (playlistData.current == -1)
            setCurrentTrack(0);
        qDebug() << "loaded" << path << songs;
        return true;
    }
    if (seen.size() == 1)
        qWarning("[%s] doesn't seem to be a valid file", qPrintable((*seen.begin()).absoluteFilePath()));
    return false;
}

bool Backend::removeTrack(int index)
{
    qWarning("%s not implemented", __FUNCTION__);
    return false;
}

bool Backend::swap(int from, int to)
{
    qWarning("%s not implemented", __FUNCTION__);
    return false;
}

bool Backend::setCWD(const QString &path)
{
    return QDir::setCurrent(path);
}

QString Backend::CWD() const
{
    return QDir::currentPath();
}

void Backend::clear()
{
    if (!playlistData.tracks.isEmpty()) {
        playlistData.tracks.clear();
        playlistData.current = -1;
        emit trackCountChanged(0);
        emit currentTrackChanged(-1, QString());
    }
}

void Backend::quit()
{
    shutdown();
    QCoreApplication::quit();
}

void Backend::sendWakeUp()
{
//    qDebug() << receivers(SIGNAL(wakeUp()));
    emit wakeUp();
}
