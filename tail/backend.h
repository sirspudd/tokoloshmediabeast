#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore>
#include <global.h>

struct RootNode;
class Backend : public QObject
{
    Q_OBJECT
public:
    enum Status {
        Uninitalized,
        Playing,
        Paused,
        Stopped
    };

    enum ProgressType {
        Seconds,
        Portion // ### 100th of a percent. Xine uses 0-65535 should we too?. Not a good name btw
    };

    enum Capability {
        NoCapabilities = 0x000,
        SupportsEqualizer = 0x001,
        SupportsRemoteFiles = 0x002
    };

    enum Event {
        NoEvent = 0,
        SongFinished,
        TrackChanged,
        SongPaused,
        PlayResumed,
        SongStopped,
        VolumeChanged,
        MuteChanged,
        EqualizerChanged,
        ProgressChanged // ### ????
    };

    bool load(const QUrl &path, bool recursive);
    virtual bool trackData(TrackData *data, const QUrl &path, int types = All) const = 0;
    virtual void shutdown() = 0;
    QList<Function> findFunctions(const QString &functionName) const;

public slots:
    Q_SCRIPTABLE virtual int capabilities() const { return None; }
    Q_SCRIPTABLE virtual bool isValid(const QUrl &url) const = 0;
    Q_SCRIPTABLE virtual void play() = 0;
    Q_SCRIPTABLE virtual void pause() = 0;
    Q_SCRIPTABLE virtual void setProgress(int type, int progress) = 0;
    Q_SCRIPTABLE virtual int progress(int type) = 0;
    Q_SCRIPTABLE virtual void stop() = 0;
    Q_SCRIPTABLE virtual bool loadUrl(const QUrl &fileName) = 0;
    Q_SCRIPTABLE virtual int status() const = 0;
    Q_SCRIPTABLE virtual int volume() const = 0;
    Q_SCRIPTABLE virtual void setVolume(int vol) = 0;
    Q_SCRIPTABLE virtual bool initBackend() = 0;
    Q_SCRIPTABLE virtual QString errorMessage() const { return QString(); }
    Q_SCRIPTABLE virtual int errorCode() const { return 0; }
    Q_SCRIPTABLE virtual void setMute(bool on) = 0;
    Q_SCRIPTABLE virtual bool isMute() const = 0;
    Q_SCRIPTABLE virtual QHash<int, int> equalizerSettings() const { return QHash<int, int>(); }
    Q_SCRIPTABLE virtual void setEqualizerSettings(const QHash<int, int> &) {}

    // playlist stuff
    Q_SCRIPTABLE TrackData trackData(int idx, int fields = All) const;
    Q_SCRIPTABLE TrackData trackData(const QUrl &path, int fields = All) const;
    Q_SCRIPTABLE int count() const;
    Q_SCRIPTABLE QString currentTrackName() const;
    Q_SCRIPTABLE int currentTrackIndex() const;
    Q_SCRIPTABLE QStringList list() const;
    Q_SCRIPTABLE QList<QUrl> tracks(int start, int count) const;
    Q_SCRIPTABLE bool setCurrentTrackIndex(int index);
    Q_SCRIPTABLE bool setCurrentTrack(const QString &name);
    Q_SCRIPTABLE int indexOfTrack(const QUrl &name) const;
    Q_SCRIPTABLE bool setCWD(const QString &path);
    Q_SCRIPTABLE QString CWD() const;
    Q_SCRIPTABLE QString playlist() const;
    Q_SCRIPTABLE void setPlaylist(const QString &file);
    Q_SCRIPTABLE void clear() { if (count() > 0) removeTracks(0, count()); }
    Q_SCRIPTABLE void quit();
    Q_SCRIPTABLE void sendWakeUp();
    Q_SCRIPTABLE void prev();
    Q_SCRIPTABLE void next();
    Q_SCRIPTABLE void crop();
    Q_SCRIPTABLE Function findFunction(const QString &functionName) const;
    Q_SCRIPTABLE QStringList functions() const;

    Q_SCRIPTABLE inline bool load(const QString &path) { return load(QUrl(path), false); }
    Q_SCRIPTABLE inline bool loadRecursively(const QString &path) { return load(QUrl(path), true); }
    Q_SCRIPTABLE bool removeTracks(int index, int count);
    Q_SCRIPTABLE bool removeTrack(int index) { return removeTracks(index, 1); }
    Q_SCRIPTABLE bool swapTrack(int from, int to);
    Q_SCRIPTABLE bool moveTrack(int from, int to);
signals:
    Q_SCRIPTABLE void wakeUp();
    Q_SCRIPTABLE void trackNames(int from, const QStringList &list);
    Q_SCRIPTABLE void currentTrackChanged(int index, const QUrl &string);

    Q_SCRIPTABLE void tracksInserted(int from, int count);
    Q_SCRIPTABLE void tracksRemoved(int from, int count);
    Q_SCRIPTABLE void trackMoved(int from, int to);
    Q_SCRIPTABLE void tracksSwapped(int from, int to);
    // need to emit this if e.g. the command line client changes the
    // volume on us. The other clients need to know to move their
    // slider etc
    Q_SCRIPTABLE void event(int type, const QList<QVariant> &data);
    Q_SCRIPTABLE void statusChanged(Status status);
    Q_SCRIPTABLE void foo(int);
private slots:
#ifdef THREADED_RECURSIVE_LOAD
    void onThreadFinished();
#endif
#ifdef Q_OS_UNIX
    void onUnixSignal(int signal);
#endif
protected:
//    enum SyncMode { ToFile, FromFile };
    bool syncToFile();
    bool syncFromFile(bool *foundInvalidSongs);
//    bool sync(SyncMode sync, bool *removedSongs);
    void addTracks(const QStringList &list);
    Backend(QObject *parent);
    virtual ~Backend();
    struct PlaylistData {
        PlaylistData() : current(-1), root(0), blockSync(false) {}
        int current;
        QFile playlist;
        QList<QUrl> tracks;
        QMap<QUrl, TrackData> cache;
        mutable RootNode *root;
        bool blockSync;
    } playlistData;
};

#endif
