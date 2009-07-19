#include "tail.h"
#include "log.h"
#include <config.h>
#include "taginterface.h"
#include "id3taginterface.h"
#ifdef Q_OS_UNIX
#include <signal.h>
#endif

struct FunctionNode
{
    ~FunctionNode() { qDeleteAll(nodes); }
    QHash<QChar, FunctionNode*> nodes;
    Function function;
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
    d.tag = new ID3TagInterface;
    QString playlistPath = Config::value<QString>("playlist");
    if (!playlistPath.isEmpty() && QFile::exists(playlistPath)) {
        d.playlist.setFileName(playlistPath);
        bool foundInvalid = false;
        syncFromFile(&foundInvalid);
        if (foundInvalid)
            syncToFile();
    } else {
        playlistPath = QString("%1/tokolosh.m3u").
                       arg(QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
        Config::setValue<QString>("playlist", playlistPath);
        d.playlist.setFileName(playlistPath);
    }
    d.current = Config::value<int>("current");
    ::fixCurrent(&d.current, d.tracks.size());
#ifdef Q_OS_UNIX
//    QCoreApplication::watchUnixSignal(SIGINT, true); // doesn't seem to work
    connect(QCoreApplication::instance(), SIGNAL(unixSignal(int)), this, SLOT(onUnixSignal(int)));
#endif
}

bool Tail::setBackend(Backend *backend)
{
    Q_ASSERT(backend);
    if (!backend->initBackend())
        return false;
    d.backend = backend;
    return true;
}


Tail::~Tail()
{
    delete d.tag;
    if (d.backend) {
        delete d.backend;
    }
    delete d.root;
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
        if (node->function.name.isEmpty()) {
            node->function = function;
        } else {
            node->function.args.append(function.args);
        }
        return;
    }
    FunctionNode *&n = node->nodes[string.at(0).toLower()];
    if (!n) {
        n = new FunctionNode;
    }
    addFunction(n, string.mid(1), function);
}

static inline QStringList recurse(const FunctionNode *node)
{
    Q_ASSERT(node);
    QStringList ret;
    foreach(const QList<int> &args, node->function.args) {
        QStringList argNames;
        foreach(int arg, args) {
            argNames.append(QMetaType::typeName(arg));
        }
        ret.append(QString("%0 %1(%2)").
                   arg(QMetaType::typeName(node->function.returnArgument)).
                   arg(node->function.name).
                   arg(argNames.join(", ")));
    }
    for (QHash<QChar, FunctionNode *>::const_iterator it = node->nodes.begin(); it != node->nodes.end(); ++it) {
        ret += ::recurse(it.value());
    }

    return ret;
}

static Function findFunction(const FunctionNode *node, const QString &name, QString *error)
{
    if (name.isEmpty()) {
        while (node->function.name.isEmpty() && node->nodes.size() == 1) {
            node = node->nodes.values().first();
        }
        if (error && node->function.name.isEmpty() && node->nodes.size() > 1) {
            *error = "Ambigous request. Could match: " + ::recurse(node).join("\n");
        }
        return node->function;
    }

    FunctionNode *n = node->nodes.value(name.at(0));
    if (!n) {
        return Function();
    }
    return findFunction(n, name.mid(1), error);
}


Function Tail::findFunction(const QString &functionName) const
{
    if (!d.root) {
        d.root = new FunctionNode;
        const QMetaObject *meta = metaObject();
        const int count = meta->methodCount();
        for (int i=0; i<count; ++i) {
            const QMetaMethod method = meta->method(i);
            if (method.attributes() & QMetaMethod::Scriptable) {
                Function func;
                func.name = QString::fromLatin1(method.signature());
                func.name.chop(func.name.size() - func.name.indexOf('('));
                func.returnArgument = QMetaType::type(method.typeName());
                QList<int> args;
                foreach(const QByteArray &parameter, method.parameterTypes()) {
                    args.append(QMetaType::type(parameter.constData()));
                }
                func.args.append(args);
                ::addFunction(d.root, func.name, func);
                QSet<QString> used;
                used.insert(func.name);
                const QString translated = tr(qPrintable(func.name)); // ### need to make sure this is translated
                if (translated.isEmpty() && translated != func.name) {
                    used.insert(translated);
                    ::addFunction(d.root, translated, func);
                }
                const QStringList aliases = Config::value<QString>(QLatin1String("Aliases/") + func.name).split(' ', QString::SkipEmptyParts);
                foreach(const QString &alias, aliases) {
                    if (!used.contains(alias)) { // ### warn about dupe alias?
                        used.insert(alias);
                        ::addFunction(d.root, alias, func);
                    }
                }
            }
        }
    }
    return ::findFunction(d.root, functionName.toLower(), &d.lastError);
}

QStringList Tail::functions() const
{
    if (!d.root) {
        findFunction(QString());
    }
    return ::recurse(d.root);
}

QString Tail::lastError() const
{
    return d.lastError;
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
        url.prepend(QString("%1. ").arg(i, width));
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

int Tail::indexOfTrack(const QString &name) const
{
    for (int i=0; i<d.tracks.size(); ++i) {
        const QUrl &url = d.tracks.at(i);
        if (url.toString().contains(name, Qt::CaseInsensitive))
            return i;
    }
    return -1;
}

TrackData Tail::trackData(const QString &song, int fields) const
{
    const int idx = indexOfTrack(song);
    return (idx == -1 ? TrackData() : trackData(idx, fields));
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
    if (fields & PlaylistIndex) {
        data.playlistIndex = index;
    }

    enum { BackendTypes = Title|TrackLength|Artist|Year|Genre|AlbumIndex };
    const uint backendTypes = fields & BackendTypes;;
    if (backendTypes) {
        d.tag->trackData(&data, d.tracks.at(index), backendTypes);
//        d.backend->trackData(&data, d.tracks.at(index), backendTypes); // ### check return value?
    }
    data.fields |= fields;
//    ::sleep(250);
    return data;
}

enum RecursiveLoadFlags {
    Recurse = 0x1,
    ResolveSymlinks = 0x2,
    IgnoreExtension = 0x4,
    Threaded = 0x8
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
{
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
    } else if (file.isFile()) {
        const QString suffix = file.suffix().toLower();
        if (suffix == "m3u") { // ## could and should use libmagic to detect all of this
            QFile f(file.absoluteFilePath());
            if (f.open(QIODevice::ReadOnly)) {
                QTextStream ts(&f);
                while (!ts.atEnd()) {
                    QString line = ts.readLine();
                    if (!QFile::exists(line)) {
                        line.prepend(file.absolutePath() + QDir::separator());
                        if (!QFile::exists(line))
                            continue;
                    }
                    const QFileInfo fi(line);
                    if (fi.isFile()) {
                        ret += recursiveLoad(fi, flags, validExtensions);
                    } else {
                        Log::log(1) << "Don't know what to do with this" << fi.absoluteFilePath();
                    }
                }
            }
        } else if  (flags & IgnoreExtension || validExtensions.contains(suffix)) {
            ret.append(file.absoluteFilePath());
        }
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
        songs = ::recursiveLoad(fileInfo, flags|Threaded, validExtensions);
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
    // ### this code does not deal well with remote m3u's yet
    const QString path = url.toLocalFile();
    if (path.isEmpty()) {
        // remote file somehow
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

QStringList Tail::tags(const QString &filename) const
{
    const int idx = indexOfTrack(filename);
    QStringList list;
    if (idx != -1) {
        const TrackData data = trackData(idx);
        const TrackInfo *info = ::trackInfos;
        while (*info != None) {
            const QVariant var = data.data(*info);
//            qDebug() << var << qVariantValue<QUrl>(var).toString() << ::trackInfoToString(*info);
            if (!var.isNull()) {
                list.append(QString("%1: \"%2\"").arg(::trackInfoToString(*info)).arg(
                                (*info == URL ? data.url.toString() : var.toString())));
            }
            ++info;
        }
    }
    return list;
}
