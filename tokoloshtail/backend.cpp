#include "backend.h"
#include "log.h"
#include <config.h>
#ifdef Q_OS_UNIX
#include <signal.h>
#endif

struct FunctionNode
{
    ~FunctionNode() { qDeleteAll(nodes); }
    QMap<QChar, FunctionNode*> nodes;
    QList<Function> functions;
};

struct RootNode : public FunctionNode
{
    QStringList all;
};

static inline void fixCurrent(int *current, int size)
{
    if (size == 0) {
        *current = -1;
    } else {
        *current = qBound(0, *current, size - 1);
    }
}

Backend::Backend(QObject *parent)
    : QObject(parent)
{
    const QString playlistPath = Config::value<QString>("playlist",
                                                        QString("%1/tokoloshhead_%2.m3u").
                                                        arg(QDesktopServices::storageLocation(QDesktopServices::TempLocation)).
                                                        arg(QCoreApplication::applicationPid()));
    Config::setValue<QString>("playlist", playlistPath);
    playlistData.playlist.setFileName(playlistPath);
    sync(FromFile);
    bool removed = false;
    for (int i=playlistData.tracks.size() - 1; i>=0; --i) {
        if (!QFile::exists(playlistData.tracks.at(i))) {
            playlistData.tracks.removeAt(i);
            removed = true;
        }
    }
    if (removed) {
        sync(ToFile);
    }
    playlistData.current = Config::value<int>("current");
    ::fixCurrent(&playlistData.current, playlistData.tracks.size());
#ifdef Q_OS_UNIX
//    QCoreApplication::watchUnixSignal(SIGINT, true); // doesn't seem to work
    connect(QCoreApplication::instance(), SIGNAL(unixSignal(int)), this, SLOT(onUnixSignal(int)));
#endif
}

Backend::~Backend()
{
    delete playlistData.root;
//    Config::setValue("playlist", playlistData.tracks);
    Config::setValue("current", playlistData.current);
}


void Backend::prev()
{
    if (!playlistData.tracks.size()) {
        return;
    }
    setCurrentTrackIndex((playlistData.current + playlistData.tracks.size() - 1)
                         % playlistData.tracks.size());
    play();
}

void Backend::next()
{
    if (!playlistData.tracks.size()) {
        return;
    }
    setCurrentTrackIndex((playlistData.current + 1) %
                         playlistData.tracks.size());
    play();
}

void Backend::crop()
{
    if (playlistData.tracks.size() <= 1)
        return;
    Q_ASSERT(playlistData.current != -1);
    playlistData.blockSync = true;
    if (playlistData.current + 1 < playlistData.tracks.size())
        removeTracks(playlistData.current + 1, playlistData.tracks.size() - 1 - playlistData.current);
    if (playlistData.current > 0)
        removeTracks(0, playlistData.current);
    playlistData.blockSync = false;
    sync(ToFile);
}

static void addFunction(FunctionNode *node, const QString &string, const Function &function)
{
    Q_ASSERT(node);
    if (string.isEmpty()) {
        node->functions.append(function);
        return;
    }
    FunctionNode *&n = node->nodes[string.at(0)];
    if (!n) {
        n = new FunctionNode;
    }
    addFunction(n, string.mid(1), function);
}

static QList<Function> findFunctions(FunctionNode *node, const QString &name)
{
    if (name.isEmpty()) {
        while (node->functions.isEmpty() && node->nodes.size() == 1) {
            node = node->nodes.values().first();
        }

        return node->functions;
    }

    FunctionNode *n = node->nodes.value(name.at(0));
    if (!n) {
        return QList<Function>();
    }
    return findFunctions(n, name.mid(1));
}


QList<Function> Backend::findFunctions(const QString &functionName) const
{
    if (!playlistData.root) {
        playlistData.root = new RootNode;
        const QMetaObject *meta = metaObject();
        const int count = meta->methodCount();
        for (int i=0; i<count; ++i) {
            const QMetaMethod method = meta->method(i);
            if (method.attributes() & QMetaMethod::Scriptable) {
                Function func;
                func.name = method.signature();
                func.name.chop(func.name.size() - func.name.indexOf('('));
//                qDebug() << method.parameterNames() << func.name << method.parameterTypes();
                foreach(const QByteArray &parameter, method.parameterTypes()) {
                    func.args.append(QMetaType::type(parameter.constData()));
                }
                ::addFunction(playlistData.root, func.name, func);
                QSet<QString> used;
                used.insert(func.name);
                const QString translated = tr(qPrintable(func.name)); // ### need to make sure this is translated
                if (translated.isEmpty() && translated != func.name) {
                    used.insert(translated);
                    ::addFunction(playlistData.root, translated, func);
                }
                const QStringList aliases = Config::value<QString>(QLatin1String("Aliases/") + func.name).split(' ', QString::SkipEmptyParts);
//                qDebug() << func.name << aliases;
                foreach(const QString &alias, aliases) {
                    if (used.contains(alias)) { // ### warn about dupe alias?
                        used.insert(alias);
                        ::addFunction(playlistData.root, alias, func);
                    }
                }
                playlistData.root->all += used.toList();
            }
        }
    }
    QList<Function> ret = ::findFunctions(playlistData.root, functionName);
    return ret;
}

Function Backend::findFunction(const QString &functionName) const
{
    return findFunctions(functionName).value(0);
}


QStringList Backend::functions() const
{
    if (!playlistData.root) {
        findFunctions(QString());
    }
    return playlistData.root->all;
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

static inline int width(int num)
{
    int count = 1;
    while (num >= 10) {
        ++count;
        num /= 10;
    }
    return count;
}

QStringList Backend::list() const
{
    QStringList ret = playlistData.tracks;
    const int width = ::width(ret.size()) + 2;
    for (int i=0; i<ret.size(); ++i) {
        QString &ref = ret[i];
        ref.prepend(QString("%1. ").arg(i + 1, width));
        if (i == playlistData.current) {
            ref[0] = '*';
        }
    }
    return ret;
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

bool Backend::setCurrentTrackIndex(int index)
{
    if (index >= 0 && index < playlistData.tracks.size()) {
        playlistData.current = index;
        const QString &filePath = playlistData.tracks.at(index);
        loadFile(filePath);
        emit currentTrackChanged(index, filePath); //, trackData(playlistData.tracks.at(index)));
        Config::setValue<int>("current", playlistData.current);
        return true;
    } else {
        return false;
    }
}

bool Backend::setCurrentTrack(const QString &name)
{
    const int idx = playlistData.tracks.indexOf(name);
    if (idx != -1) {
        setCurrentTrackIndex(idx);
        return true;
    }
    return false;
}

int Backend::indexOfTrack(const QString &name) const
{
    return playlistData.tracks.indexOf(name);
}

TrackData Backend::trackData(const QString &filepath, int fields) const
{
    const int index = indexOfTrack(filepath);
    if (index == -1) {
        qWarning("I don't have %s in my list of files", qPrintable(filepath));
        return TrackData();
    }
    return trackData(index, fields);
}

TrackData Backend::trackData(int index, int fields) const
{
    if (index < 0 || index >= playlistData.tracks.size()) {
        qWarning("Invalid index %d, needs to be between 0-%d", index, playlistData.tracks.size() - 1);
        return TrackData();
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
//    ::sleep(250);
    return data;
}

enum RecursiveLoadFlags {
    Recurse = 0x1,
    ResolveSymlinks = 0x2,
    IgnoreExtension = 0x4
};

static inline bool resolveSymlink(QFileInfo *fi)
{
    QSet<QFileInfo> seen;
    while (fi->isSymLink()) {
        if (seen.contains(*fi)) {
            qWarning("Recursive symlink detected %s", qPrintable(fi->absoluteFilePath()));
            return false;
        }
        seen.insert(*fi);
        *fi = QFileInfo(fi->readLink());
    }
    return true;
}

static inline uint qHash(const QFileInfo &fi) { return qHash(fi.absoluteFilePath()); }
static inline QStringList recursiveLoad(QFileInfo file, uint flags, const QSet<QString> &validExtensions)
{ // if backend is 0 it's being run in a thread
    QStringList ret;
    if (file.isSymLink() && (!(flags & ResolveSymlinks) || !::resolveSymlink(&file))) {
        return ret;
    }
    Q_ASSERT(!file.isSymLink());
    if (file.isDir()) {
        const QDir dir(file.absoluteFilePath());
        QDir::Filters filter = QDir::Files;
        if (flags & Recurse)
            filter |= QDir::NoDotAndDotDot|QDir::Dirs;
        const QFileInfoList list = dir.entryInfoList(filter);
        foreach(const QFileInfo &f, list) {
            Q_ASSERT(!f.isDir() || flags & Recurse);
            ret += ::recursiveLoad(f, flags, validExtensions);
        }
    } else if (file.isFile() && (flags & IgnoreExtension || validExtensions.contains(file.suffix().toLower()))) {
        ret.append(file.absoluteFilePath());
    }
    return ret;
}

#ifdef THREADED_RECURSIVE_LOAD
class DirectoryThread : public QThread
{
    Q_OBJECT
public:
    DirectoryThread(const QFileInfo &f, uint flg, const QSet<QString> &extensions, QObject *parent)
        : QThread(parent), fileInfo(f), flags(flg), validExtensions(extensions)
    {
    }

    virtual void run()
    {
        songs = ::recursiveLoad(fileInfo, flags, validExtensions);
    }

    const QFileInfo fileInfo;
    const uint flags;
    const QSet<QString> validExtensions;
    QStringList songs;
};

#include "backend.moc"

void Backend::onThreadFinished()
{
    DirectoryThread *thread = qobject_cast<DirectoryThread*>(sender());
    addTracks(thread->songs);
    thread->deleteLater();
}

#endif

void Backend::addTracks(const QStringList &list)
{
    QStringList valid;
    static const bool trustExtension = Config::isEnabled("trustextension", true);
    foreach(const QString &file, list) {
        if (trustExtension || isValid(file)) {
            valid.append(file);
        }
    }
    if (!valid.isEmpty()) {
        playlistData.tracks.append(valid);
//         if (playlistData.playlist.isWritable()) {
//             QTextStream ts(&playlistData.playlist);
//             for (int i=0; i<valid.size(); ++i) {
//                 ts << valid.at(i) << endl;
//             }
//         } else {
            sync(ToFile);
//        }
        emit tracksInserted(playlistData.tracks.size() - valid.size(), valid.size());
        if (playlistData.current == -1) {
            setCurrentTrackIndex(0);
        }
    }
}

bool Backend::load(const QString &path, bool recurse)
{
    static const QSet<QString> validExtensions = Config::value<QStringList>("extensions",
                                                                            (QStringList() << "mp3" << "ogg" << "flac"
                                                                             << "acc" << "m4a" << "mp4")).toSet();
    static const bool resolveSymlinks = Config::isEnabled("resolvesymlinks", true);
    static const bool ignoreExtension = Config::isEnabled("ignoreextension", false);

    QFileInfo file(path);
    if (file.isSymLink() && (!resolveSymlinks || !::resolveSymlink(&file))) {
        return false;
    }
    uint flags = recurse ? Recurse : 0;
    if (resolveSymlinks)
        flags |= ResolveSymlinks;
    if (ignoreExtension)
        flags |= IgnoreExtension;

    Log::log(5) << path << recurse << file.absoluteFilePath();
    if (!file.exists()) {
        qWarning("%s doesn't seem to exist", qPrintable(file.absoluteFilePath()));
        return false;
    }

#ifdef THREADED_RECURSIVE_LOAD
    if (recurse && file.isDir()) {
        DirectoryThread *thread = new DirectoryThread(file, flags, validExtensions, this);
        connect(thread, SIGNAL(finished()), this, SLOT(onThreadFinished()));
        thread->start();
        return true;
    }
#endif
    const QStringList songs = ::recursiveLoad(file, flags, validExtensions);
    addTracks(songs);
    if (file.isFile() && songs.isEmpty()) {
        Log::log(0) << file.absoluteFilePath() << "doesn't seem to be a valid file";
        return false;
    }
    return true;
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
    if (playlistData.tracks.isEmpty()) {
        playlistData.current = -1;
        action = EmitCurrentChanged;
    }
    switch (action) {
    case Nothing:
        break;
    case EmitCurrentChanged:
        emit currentTrackChanged(playlistData.current, playlistData.tracks.value(playlistData.current));
        break;
    case Next:
        next();
        break;
    }
    if (!playlistData.blockSync)
        sync(ToFile);
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
    sync(ToFile);
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
    sync(ToFile);
    return true;
}

#ifdef Q_OS_UNIX
void Backend::onUnixSignal(int)
{
    Config::setValue("playlist", playlistData.tracks);
    Config::setValue("current", playlistData.current);
    exit(0);
}

#endif

QString Backend::playlist() const
{
    return playlistData.playlist.fileName();
}

void Backend::setPlaylist(const QString &file)
{
    playlistData.playlist.setFileName(file);
    sync(FromFile);
}

bool Backend::sync(SyncMode sync)
{
    if (sync == ToFile) {
        qDebug() << "syncing to file" << QFileInfo(playlistData.playlist.fileName()).absoluteFilePath();
//         if (playlistData.playlist.isWritable())
//             playlistData.playlist.remove();
//        if (!playlistData.playlist.open(QIODevice::ReadWrite)) {
        if (!playlistData.playlist.open(QIODevice::WriteOnly)) {
            Log::log(0) << "Can't open" << QFileInfo(playlistData.playlist).absoluteFilePath() << "for writing";
            return false;
        }
        QTextStream ts(&playlistData.playlist);
        foreach(const QString &file, playlistData.tracks) {
            ts << file;
            ts << endl;
        }
        playlistData.playlist.close();
    } else {
        const int idx = playlistData.current;
        const QString track = playlistData.tracks.value(idx);
//        if (!playlistData.playlist.isReadable()) {
        if (!playlistData.playlist.open(QIODevice::ReadOnly)) {
            Log::log(0) << "Can't open" << QFileInfo(playlistData.playlist).absoluteFilePath() << "for reading";
            return false;
        }
        playlistData.tracks.clear();
        QTextStream ts(&playlistData.playlist);
        while (!ts.atEnd()) {
            playlistData.tracks.append(ts.readLine());
        }
        ::fixCurrent(&playlistData.current, playlistData.tracks.size());
        const QString currentTrack = currentTrackName();
        if (playlistData.current != idx || currentTrack != track) {
            emit currentTrackChanged(playlistData.current, currentTrack);
        }
        playlistData.playlist.close();
    }
    return true;
}
