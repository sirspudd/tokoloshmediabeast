#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore>
#include <global.h>
#include "playlist.h"

struct BackendPrivate : public PlaylistPrivate
{
};

class Backend : public Playlist
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
    static Backend *instance();
public slots:
    virtual int capabilities() const { return None; }
    virtual void shutdown() = 0;
    virtual QVariant field(const QString &fileName, int field) const = 0;
    virtual bool isValid(const QString &fileName) const = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void setProgress(int type, int progress) = 0;
    virtual int progress(int type) = 0;
    virtual void stop() = 0;
    virtual bool load(const QString &fileName) = 0;
    virtual QString currentTrack() const = 0;
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
signals:
    // need to emit this if e.g. the command line client changes the
    // volume on us. The other clients need to know to move their
    // slider etc
    void event(int type, const QVariant &data);
    void statusChanged(Status status);
    void trackChanged(const QString &string);
protected:
    Backend(BackendPrivate &dd, QObject *parent);
    virtual ~Backend() {}
private:
    BackendPrivate *d;
    static Backend *inst;
};

#endif
