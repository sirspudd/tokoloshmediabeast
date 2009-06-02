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
public slots:
    virtual int capabilities() const { return None; }
    virtual void shutdown() = 0;
    void sendWakeUp() { qWarning("%s not implemented", __FUNCTION__); }
    virtual bool isValid(const QString &fileName) const = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    void prev();
    void next();
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
    QStringList tracks(int start, int count) const;
    bool setCurrentTrack(int index);
    bool setCurrentTrack(const QString &name);
    int indexOfTrack(const QString &name) const;
    bool requestTrackData(const QString &filepath, uint trackInfo = All);
    bool requestTracknames(int from, int size);
    bool swap(int from, int to);
    inline bool load(const QString &path) { return load(path, false); }
    inline bool loadRecursively(const QString &path) { return load(path, true); }
    bool removeTrack(int index);
    bool setCWD(const QString &path);
    QString CWD() const;
    void clear();
signals:
    void swapped(int from, int to);
    void trackData(const QString &path, const QVariant &data);
    void trackNames(int from, const QStringList &list);
    void currentTrackChanged(int index, const QString &string);
    void trackCountChanged(int count);
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
        // QMap<QString, QByteArray> cachedData; // ### ???
        QSet<QString> *seenPaths;
    } playlistData;
private:
    static Backend *inst;
};

#endif
