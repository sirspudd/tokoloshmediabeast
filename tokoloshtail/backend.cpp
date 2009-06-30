#include "backend.h"
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

Backend *Backend::inst = 0;
Backend *Backend::instance()
{
    return inst;
}

Backend::Backend()
{
    startTimer(1000);
    Q_ASSERT(!inst);
    inst = this;
    delete playlistData.root;
    playlistData.tracks = Config::value<QStringList>("playlist", QStringList());
    for (int i=playlistData.tracks.size() - 1; i>=0; --i) {
        if (!QFile::exists(playlistData.tracks.at(i))) {
            playlistData.tracks.removeAt(i);
        }
    }
    playlistData.current = qMin(playlistData.tracks.size() - 1, Config::value<int>("current"));
#ifdef Q_OS_UNIX
//    QCoreApplication::watchUnixSignal(SIGINT, true); // doesn't seem to work
    connect(QCoreApplication::instance(), SIGNAL(unixSignal(int)), this, SLOT(onUnixSignal(int)));
#endif
}

Backend::~Backend()
{
    Config::setValue("playlist", playlistData.tracks);
    Config::setValue("current", playlistData.current);
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

void Backend::crop(int index)
{
    if (playlistData.tracks.size() <= 1)
        return;
    Q_ASSERT(playlistData.current != -1);
    if (index == -1)
        index = playlistData.current;
    if (index < 0 || index >= playlistData.tracks.size()) {
        qWarning("crop. Invalid index %d (0-%d)", index, playlistData.tracks.size());
        return;
    }
    if (index + 1 < playlistData.tracks.size())
        removeTracks(index + 1, playlistData.tracks.size() - 1 - index);
    if (index > 0)
        removeTracks(0, index);
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

QStringList Backend::list() const
{
    QStringList ret = playlistData.tracks;
    for (int i=0; i<ret.size(); ++i) {
        QString &ref = ret[i];
        ref.prepend(QString("%1 ").arg(i + 1));
    }
    qDebug() << ret;
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
    qDebug() << "trackData" << index << fields;
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
    qDebug() << path << recurse << fi.absoluteFilePath();
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

#ifdef Q_OS_UNIX
void Backend::onUnixSignal(int)
{
    qDebug("%s %d: void Backend::onUnixSignal(int)", __FILE__, __LINE__);
    Config::setValue("playlist", playlistData.tracks);
    Config::setValue("current", playlistData.current);
    exit(0);
}

#endif
