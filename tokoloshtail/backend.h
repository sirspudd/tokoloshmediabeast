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
    virtual int capabilities() const { return None; }
    virtual bool isValid(const QString &fileName) const = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void setProgress(int type, int progress) = 0;
    virtual int progress(int type) = 0;
    virtual void stop() = 0;
    virtual bool loadFile(const QString &fileName) = 0;
    virtual int status() const = 0;
    virtual int volume() const = 0;
    virtual void setVolume(int vol) = 0;
    virtual bool initBackend() = 0;
    virtual QString errorMessage() const { return QString(); }
    virtual int errorCode() const { return 0; }
    virtual void setMute(bool on) = 0;
    virtual bool isMute() const = 0;
    virtual QByteArray equalizerSettings() const { return QByteArray(); }
    virtual void setEqualizerSettings(const QByteArray &) {}

    // do all of this through a generic QVariant value(int
    // type)/setValue(int type, QVariant) interface so I can be binary
    // compatible etc? Would anyone in their right mind need us to be
    // bc?

    // playlist stuff
    int count() const;
    QString currentTrackName() const;
    int currentTrackIndex() const;
    QStringList list() const { return playlistData.tracks; }
    QStringList tracks(int start, int count) const;
    bool setCurrentTrack(int index);
    bool setCurrentTrack(const QString &name);
    int indexOfTrack(const QString &name) const;
    bool requestTrackData(const QString &filepath, uint trackInfo = All);
    bool requestTrackData(int index, uint trackInfo = All);
    bool requestTracknames(int from, int size);
    bool setCWD(const QString &path);
    QString CWD() const;
    void clear() { if (count() > 0) removeTracks(0, count()); }
    void quit();
    void sendWakeUp();
    void prev();
    void next();
    void ping() {}

    inline bool load(const QString &path) { return load(path, false); }
    inline bool loadRecursively(const QString &path) { return load(path, true); }
    bool removeTracks(int index, int count);
    bool removeTrack(int index) { return removeTracks(index, 1); }
    bool swapTrack(int from, int to);
    bool moveTrack(int from, int to);

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
