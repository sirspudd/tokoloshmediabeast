#include "tail.h"
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

Tail::Tail(QObject *parent)
    : QObject(parent)
{
    QString playlistPath = Config::value<QString>("playlist");
    if (!playlistPath.isEmpty()) {
        bool foundInvalid = false;
        syncFromFile(&foundInvalid);
        if (foundInvalid)
            syncToFile();
    } else {
        playlistPath = QString("%1/tokolosh.m3u").
                       arg(QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
        Config::setValue<QString>("playlist", playlistPath);
    }
    d.playlist.setFileName(playlistPath);
    d.current = Config::value<int>("current");
    ::fixCurrent(&d.current, d.tracks.size());
#ifdef Q_OS_UNIX
//    QCoreApplication::watchUnixSignal(SIGINT, true); // doesn't seem to work
    connect(QCoreApplication::instance(), SIGNAL(unixSignal(int)), this, SLOT(onUnixSignal(int)));
#endif
}

bool Tail::setBackend(Backend *backend)
{
    qDebug() << "setbackend" << backend;
    Q_ASSERT(backend);
    if (!backend->initBackend())
        return false;
    d.backend = backend;
    return true;
}


Tail::~Tail()
{
    if (d.backend) {
        qDebug("%s %d: if (d.backend) {", __FILE__, __LINE__);
//        delete d.backend;
        qDebug("%s %d: delete d.backend;", __FILE__, __LINE__);
    }
    qDebug("%s %d: delete d.root;", __FILE__, __LINE__);
    delete d.root;
    qDebug("%s %d: delete d.root;", __FILE__, __LINE__);
//    Config::setValue("playlist", d.tracks);
    qDebug("%s %d: Config::setValue(\"current\", d.current);", __FILE__, __LINE__);
}


void Tail::prev()
{
    if (!d.tracks.size()) {
        return;
    }
    setCurrentTrackIndex((d.current + d.tracks.size() - 1)
                         % d.tracks.size());
    play();
}

void Tail::next()
{
    if (!d.tracks.size()) {
        return;
    }
    setCurrentTrackIndex((d.current + 1) %
                         d.tracks.size());
    play();
}

void Tail::crop()
{
    if (d.tracks.size() <= 1)
        return;
    Q_ASSERT(d.current != -1);
    d.blockSync = true;
    if (d.current + 1 < d.tracks.size())
        removeTracks(d.current + 1, d.tracks.size() - 1 - d.current);
    if (d.current > 0)
        removeTracks(0, d.current);
    d.blockSync = false;
    syncToFile();
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


QList<Function> Tail::findFunctions(const QString &functionName) const
{
    if (!d.root) {
        d.root = new RootNode;
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
                ::addFunction(d.root, func.name, func);
                QSet<QString> used;
                used.insert(func.name);
                const QString translated = tr(qPrintable(func.name)); // ### need to make sure this is translated
                if (translated.isEmpty() && translated != func.name) {
                    used.insert(translated);
                    ::addFunction(d.root, translated, func);
                }
                const QStringList aliases = Config::value<QString>(QLatin1String("Aliases/") + func.name).split(' ', QString::SkipEmptyParts);
//                qDebug() << func.name << aliases;
                foreach(const QString &alias, aliases) {
                    if (used.contains(alias)) { // ### warn about dupe alias?
                        used.insert(alias);
                        ::addFunction(d.root, alias, func);
                    }
                }
                d.root->all += used.toList();
            }
        }
    }
    QList<Function> ret = ::findFunctions(d.root, functionName);
    return ret;
}

Function Tail::findFunction(const QString &functionName) const
{
    return findFunctions(functionName).value(0);
}


QStringList Tail::functions() const
{
    if (!d.root) {
        findFunctions(QString());
    }
    return d.root->all;
}


int Tail::count() const
{
    return d.tracks.size();
}

QString Tail::currentTrackName() const
{
    return QFileInfo(d.tracks.value(d.current).path()).fileName();
}

int Tail::currentTrackIndex() const
{
    return d.current;
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

QStringList Tail::list() const
{
    QStringList ret;
    // QList should have a reserve(int)
    const int width = ::width(d.tracks.size()) + 2;
    for (int i=0; i<d.tracks.size(); ++i) {
        QString url = d.tracks.at(i).toString();
        url.prepend(QString("%1. ").arg(i + 1, width));
        if (i == d.current) {
            url[0] = '*';
        }
        ret.append(url);
    }
    return ret;
}


template <typename T>
static inline bool tracks(const QList<T> &list, int from, int size, QList<T> *out)
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


QList<QUrl> Tail::tracks(int start, int count) const
{
    QList<QUrl> ret;
    ::tracks<QUrl>(d.tracks, start, count, &ret);
    return ret;
}

bool Tail::setCurrentTrackIndex(int index)
{
    if (index >= 0 && index < d.tracks.size()) {
        if (index == d.current) // restart???
            return true;
        d.current = index;
        const QUrl &url = d.tracks.at(index);
        loadUrl(url);
        emit currentTrackChanged(index, url); //, trackData(d.tracks.at(index)));
        Config::setValue<int>("current", d.current);
        return true;
    } else {
        return false;
    }
}

bool Tail::setCurrentTrack(const QString &name)
{
    const int idx = d.tracks.indexOf(name);
    if (idx != -1) {
        setCurrentTrackIndex(idx);
        return true;
    }
    return false;
}

int Tail::indexOfTrack(const QUrl &url) const
{
    return d.tracks.indexOf(url);
}

TrackData Tail::trackData(const QUrl &url, int fields) const
{
    const int index = indexOfTrack(url);
    if (index == -1) {
        qWarning("I don't have %s in my list of files", qPrintable(url.toString()));
        return TrackData();
    }
    return trackData(index, fields);
}

TrackData Tail::trackData(int index, int fields) const
{
    if (index < 0 || index >= d.tracks.size()) {
        qWarning("Invalid index %d, needs to be between 0-%d", index, d.tracks.size() - 1);
        return TrackData();
    }
    // ### this should maybe cache data
    TrackData data;
    if (fields & URL) {
        data.url = d.tracks.at(index);
    }
    if (fields & PlaylistIndex)
        data.playlistIndex = index;

    enum { TailTypes = Title|TrackLength|Artist|Year|Genre|AlbumIndex };
    const uint backendTypes = fields & TailTypes;
    if (backendTypes) {
        d.backend->trackData(&data, d.tracks.at(index), backendTypes); // ### check return value?
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

void Tail::onThreadFinished()
{
    DirectoryThread *thread = qobject_cast<DirectoryThread*>(sender());
    addTracks(thread->songs);
    thread->deleteLater();
}

#endif

void Tail::addTracks(const QStringList &list)
{
    QList<QUrl> valid;
    static const bool trustExtension = Config::isEnabled("trustextension", true);
    foreach(const QString &file, list) {
        if (trustExtension || isValid(file)) {
            valid.append(file);
        }
    }
    if (!valid.isEmpty()) {
        d.tracks.append(valid);
//         if (d.playlist.isWritable()) {
//             QTextStream ts(&d.playlist);
//             for (int i=0; i<valid.size(); ++i) {
//                 ts << valid.at(i) << endl;
//             }
//         } else {
        syncToFile();
//        }
        emit tracksInserted(d.tracks.size() - valid.size(), valid.size());
        if (d.current == -1) {
            setCurrentTrackIndex(0);
        }
    }
}

bool Tail::load(const QUrl &url, bool recurse)
{
    const QString path = url.toLocalFile();
    if (path.isEmpty()) {
        addTracks(QStringList() << url.toString());
        // ugly
        return true;
    }
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

bool Tail::setCWD(const QString &path)
{
    return QDir::setCurrent(path);
}

QString Tail::CWD() const
{
    return QDir::currentPath();
}


void Tail::quit()
{
    if (d.backend)
        d.backend->shutdown();
    QCoreApplication::quit();
}

void Tail::sendWakeUp()
{
    emit wakeUp();
}

bool Tail::removeTracks(int index, int count)
{
    const int size = d.tracks.size();
    if (index < 0 || index >= size || index + count > size || count <= 0) {
        qWarning("removeTracks invalid arguments %d %d count %d", index, count, size);
        return false;
    }
    enum Action { Nothing, EmitCurrentChanged, Next } action = Nothing;
    if (d.current >= index) {
        if (index + count <= d.current) { // current song is still in list, only index changed
            d.current -= count;
            action = EmitCurrentChanged;
        } else { // current song was removed, skip to next, we could have shuffle on
            action = Next;
        }
    }
    if (!d.cache.isEmpty()) {
        for (int i=index; i<index + count; ++i) {
            const QUrl &track = d.tracks.at(i);
            d.cache.remove(track);
        }
    }

    const QList<QUrl>::iterator it = d.tracks.begin() + index;
    d.tracks.erase(it, it + count);
    emit tracksRemoved(index, count);
    if (d.tracks.isEmpty()) {
        d.current = -1;
        action = EmitCurrentChanged;
    }
    switch (action) {
    case Nothing:
        break;
    case EmitCurrentChanged:
        emit currentTrackChanged(d.current, d.tracks.value(d.current));
        break;
    case Next:
        next();
        break;
    }
    if (!d.blockSync) {
        syncToFile();
    }
    return true;
}

bool Tail::swapTrack(int from, int to)
{
    const int size = d.tracks.size();
    if (from < 0 || from >= size || to < 0 || to >= size) {
        qWarning("swapTracks invalid arguments %d %d count %d", from, to, size);
        return false;
    }

    d.tracks.swap(from, to);
    emit tracksSwapped(from, to);
    syncToFile();
    return true;
}

bool Tail::moveTrack(int from, int to)
{
    const int size = d.tracks.size();
    if (from < 0 || from >= size || to < 0 || to >= size) {
        qWarning("moveTrack invalid arguments %d %d count %d", from, to, size);
        return false;
    }

    d.tracks.move(from, to);
    emit trackMoved(from, to);
    syncToFile();
    return true;
}

#ifdef Q_OS_UNIX
void Tail::onUnixSignal(int)
{
    Config::setValue("playlist", d.tracks);
    Config::setValue("current", d.current);
    exit(0);
}

#endif

QString Tail::playlist() const
{
    return d.playlist.fileName();
}

void Tail::setPlaylist(const QString &file)
{
    d.playlist.setFileName(file);
    syncToFile();
}

bool Tail::syncFromFile(bool *foundInvalidSongs)
{
    Q_ASSERT(foundInvalidSongs);
    *foundInvalidSongs = false;
    const int idx = d.current;
    const QUrl track = d.tracks.value(idx);
    if (!d.playlist.open(QIODevice::ReadOnly)) {
        Log::log(0) << "Can't open" << QFileInfo(d.playlist).absoluteFilePath() << "for reading";
        return false;
    }
    d.tracks.clear();
    QTextStream ts(&d.playlist);
    while (!ts.atEnd()) {
        const QUrl url = ts.readLine();
        // these should be urls
        const QString filePath = url.toLocalFile();
        if (filePath.isEmpty() || QFile::exists(filePath)) {
            d.tracks.append(url);
        } else {
            *foundInvalidSongs = true;
        }
    }
    ::fixCurrent(&d.current, d.tracks.size());
    const QUrl currentTrack = d.tracks.value(d.current);
    if (d.current != idx || currentTrack != track) {
        emit currentTrackChanged(d.current, currentTrack);
    }
    d.playlist.close();
    return true;
}

bool Tail::syncToFile()
{
    Log::log(10) << "syncing to file" << QFileInfo(d.playlist.fileName()).absoluteFilePath();
//         if (d.playlist.isWritable())
//             d.playlist.remove();
//        if (!d.playlist.open(QIODevice::ReadWrite)) {
    if (!d.playlist.open(QIODevice::WriteOnly)) {
        Log::log(0) << "Can't open" << QFileInfo(d.playlist).absoluteFilePath() << "for writing";
        return false;
    }
    QTextStream ts(&d.playlist);
    foreach(const QUrl &url, d.tracks) {
        ts << url.toString();
        ts << endl;
    }
    d.playlist.close();
    return true;
}

