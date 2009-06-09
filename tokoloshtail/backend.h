#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore>
#include <global.h>

class Backend : public QObject
{
    Q_OBJECT
public:
    enum Status {
        Uninitalized,
        Playing,
        Idle
    };

    enum ProgressType {
        Seconds,
        Portion // ### 100th of a percent. Xine uses 0-65535 should we too?. Not a good name btw
    };

    enum Capability {
        NoCapabilities = 0x000,
        SupportsEqualizer = 0x001
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

    bool load(const QString &path, bool recursive);
    static Backend *instance();
    virtual bool trackData(TrackData *data, const QString &path, uint types = All) const = 0;
    virtual void shutdown() = 0;
public slots:
    Q_SCRIPTABLE virtual int capabilities() const { return None; }
    Q_SCRIPTABLE virtual bool isValid(const QString &fileName) const = 0;
    Q_SCRIPTABLE virtual void play() = 0;
    Q_SCRIPTABLE virtual void pause() = 0;
    Q_SCRIPTABLE virtual void setProgress(int type, int progress) = 0;
    Q_SCRIPTABLE virtual int progress(int type) = 0;
    Q_SCRIPTABLE virtual void stop() = 0;
    Q_SCRIPTABLE virtual bool loadFile(const QString &fileName) = 0;
    Q_SCRIPTABLE virtual int status() const = 0;
    Q_SCRIPTABLE virtual int volume() const = 0;
    Q_SCRIPTABLE virtual void setVolume(int vol) = 0;
    Q_SCRIPTABLE virtual bool initBackend() = 0;
    Q_SCRIPTABLE virtual QString errorMessage() const { return QString(); }
    Q_SCRIPTABLE virtual int errorCode() const { return 0; }
    Q_SCRIPTABLE virtual void setMute(bool on) = 0;
    Q_SCRIPTABLE virtual bool isMute() const = 0;
    Q_SCRIPTABLE virtual QByteArray equalizerSettings() const { return QByteArray(); }
    Q_SCRIPTABLE virtual void setEqualizerSettings(const QByteArray &) {}

    // do all of this through a generic QVariant value(int
    // type)/setValue(int type, QVariant) interface so I can be binary
    // compatible etc? Would anyone in their right mind need us to be
    // bc?

    // playlist stuff
    Q_SCRIPTABLE int count() const;
    Q_SCRIPTABLE QString currentTrackName() const;
    Q_SCRIPTABLE int currentTrackIndex() const;
    Q_SCRIPTABLE QStringList list() const { return playlistData.tracks; }
    Q_SCRIPTABLE QStringList tracks(int start, int count) const;
    Q_SCRIPTABLE bool setCurrentTrack(int index);
    Q_SCRIPTABLE bool setCurrentTrack(const QString &name);
    Q_SCRIPTABLE int indexOfTrack(const QString &name) const;
    Q_SCRIPTABLE bool requestTrackData(const QString &filepath, uint trackInfo = All);
    Q_SCRIPTABLE bool requestTrackData(int index, uint trackInfo = All);
    Q_SCRIPTABLE bool requestTracknames(int from, int size);
    Q_SCRIPTABLE bool setCWD(const QString &path);
    Q_SCRIPTABLE QString CWD() const;
    Q_SCRIPTABLE void clear() { if (count() > 0) removeTracks(0, count()); }
    Q_SCRIPTABLE void quit();
    Q_SCRIPTABLE void sendWakeUp();
    Q_SCRIPTABLE void prev();
    Q_SCRIPTABLE void next();
    Q_SCRIPTABLE void ping() {}

    Q_SCRIPTABLE inline bool load(const QString &path) { return load(path, false); }
    Q_SCRIPTABLE inline bool loadRecursively(const QString &path) { return load(path, true); }
    Q_SCRIPTABLE bool removeTracks(int index, int count);
    Q_SCRIPTABLE bool removeTrack(int index) { return removeTracks(index, 1); }
    Q_SCRIPTABLE bool swapTrack(int from, int to);
    Q_SCRIPTABLE bool moveTrack(int from, int to);
signals:
    void wakeUp();
    void trackData(const QVariant &variant);
    void trackNames(int from, const QStringList &list);
    void currentTrackChanged(int index, const QString &string);

    void tracksInserted(int from, int count);
    void tracksRemoved(int from, int count);
    void trackMoved(int from, int to);
    void tracksSwapped(int from, int to);
    // need to emit this if e.g. the command line client changes the
    // volume on us. The other clients need to know to move their
    // slider etc
    void event(int type, const QVariant &data);
    void statusChanged(Status status);
protected:
    Backend();
    virtual ~Backend() {}
    struct PlaylistData {
        PlaylistData() : current(-1), seenPaths(0) {}
        int current;
        QStringList tracks;
        QMap<QString, TrackData> cache;
        QSet<QString> *seenPaths;
    } playlistData;
private:
    static Backend *inst;
};

#endif
