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
    startTimer(500);
}

void Backend::prev()
{
    if (!playlistData.tracks.size()) {
        return;
    }
    setCurrentTrack((playlistData.current +
                     playlistData.tracks.size() - 1) %
                        playlistData.tracks.size());
    play();
}

void Backend::next()
{
    if (!playlistData.tracks.size()) {
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

QByteArray Backend::trackData(const QString &filepath, int fields) const
{
    const int index = indexOfTrack(filepath);
    if (index == -1) {
        qWarning("I don't have %s in my list of files", qPrintable(filepath));
        return false;
    }
    return trackData(index, fields);
}

QByteArray Backend::trackData(int index, int fields) const
{
    if (index < 0 || index >= playlistData.tracks.size()) {
        qWarning("Invalid index %d, needs to be between 0-%d", index, playlistData.tracks.size() - 1);
        return QByteArray();
    }
    // ### this should maybe cache data
    TrackData data;
    if (fields & FilePath) {
        data.path = playlistData.tracks.at(index);
    }
    if (fields & PlaylistIndex)
        data.playlistIndex = index;

    enum { BackendTypes = Title|TrackLength|Artist|Year|Genre|AlbumIndex };
    const uint backendTypes = fields & BackendTypes;
    if (backendTypes) {
        trackData(&data, playlistData.tracks.at(index), backendTypes); // ### check return value?
    }
    data.fields |= fields;
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << data;
    return ba;
//    return qVariantFromValue(data);
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
        QDir::Filters filter = QDir::Files;
        if (recurse)
            filter |= QDir::NoDotAndDotDot|QDir::Dirs;
        const QFileInfoList list = dir.entryInfoList(filter);
        foreach(const QFileInfo &f, list) {
            Q_ASSERT(!f.isDir() || recurse);
            ret += ::recursiveLoad(f, recurse, seen);
        }
    } else if (file.isFile()) {
        const QString path = file.absoluteFilePath();
        static QPointer<Backend> backend = Backend::instance(); // could just be a member function
        if (backend->isValid(path)) {
            ret.append(path);
            // ### should I load song name here? Probably. Checking if
            // ### it's valid probably means parsing header anyway
        }
    }
    return ret;
}

bool Backend::load(const QString &path, bool recurse)
{
    const QFileInfo fi(path);
    if (!fi.exists()) {
        qWarning("%s doesn't seem to exist", qPrintable(path));
        return false;
    }
    QSet<QFileInfo> seen;
    const QStringList songs = ::recursiveLoad(path, recurse, &seen);
    if (!songs.isEmpty()) {
        playlistData.tracks.append(songs);
        emit tracksInserted(playlistData.tracks.size() - songs.size(), songs.size());
        if (playlistData.current == -1)
            setCurrentTrack(0);
        return true;
    }
    if (seen.size() == 1 && songs.isEmpty())
        qWarning("[%s] doesn't seem to be a valid file", qPrintable((*seen.begin()).absoluteFilePath()));
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


void Backend::quit()
{
    shutdown();
    QCoreApplication::quit();
}

void Backend::sendWakeUp()
{
    emit wakeUp();
}

bool Backend::removeTracks(int index, int count)
{
    const int size = playlistData.tracks.size();
    if (index < 0 || index >= size || index + count > size || count <= 0) {
        qWarning("removeTracks invalid arguments %d %d count %d", index, count, size);
        return false;
    }
    enum Action { Nothing, EmitCurrentChanged, Next } action = Nothing;
    if (playlistData.current >= index) {
        if (index + count <= playlistData.current) { // current song is still in list, only index changed
            playlistData.current -= count;
            action = EmitCurrentChanged;
        } else { // current song was removed, skip to next, we could have shuffle on
            action = Next;
        }
    }
    if (!playlistData.cache.isEmpty()) {
        for (int i=index; i<index + count; ++i) {
            const QString &track = playlistData.tracks.at(i);
            playlistData.cache.remove(track);
        }
    }

    const QStringList::iterator it = playlistData.tracks.begin() + index;
    playlistData.tracks.erase(it, it + count);
    emit tracksRemoved(index, count);
    switch (action) {
    case Nothing:
        break;
    case EmitCurrentChanged:
        emit currentTrackChanged(playlistData.current, playlistData.tracks.at(playlistData.current));
        break;
    case Next:
        next();
        break;
    }
    return true;
}

bool Backend::swapTrack(int from, int to)
{
    const int size = playlistData.tracks.size();
    if (from < 0 || from >= size || to < 0 || to >= size) {
        qWarning("swapTracks invalid arguments %d %d count %d", from, to, size);
        return false;
    }

    playlistData.tracks.swap(from, to);
    emit tracksSwapped(from, to);
    return true;
}

bool Backend::moveTrack(int from, int to)
{
    const int size = playlistData.tracks.size();
    if (from < 0 || from >= size || to < 0 || to >= size) {
        qWarning("moveTrack invalid arguments %d %d count %d", from, to, size);
        return false;
    }

    playlistData.tracks.move(from, to);
    emit trackMoved(from, to);
    return true;
}
