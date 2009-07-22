#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore>
#include <global.h>

class Tail;
class Backend
{
public:
    virtual ~Backend()
    {
    }
    enum Status {
        Uninitalized,
        Playing,
        Paused,
        Stopped,
        Undefined
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

    virtual bool trackData(TrackData *data, const QUrl &path, int types = All) const = 0;
    virtual void shutdown() = 0;
    virtual int capabilities() const { return None; }
    virtual bool isValid(const QUrl &url) const = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void setProgress(int type, int progress) = 0;
    virtual int progress(int type) = 0;
    virtual void stop() = 0;
    virtual bool loadUrl(const QUrl &fileName) = 0;
    virtual int status() const = 0;
    virtual int volume() const = 0;
    virtual void setVolume(int vol) = 0;
    virtual bool initBackend() = 0;
    virtual QString errorMessage() const { return QString(); }
    virtual int errorCode() const { return 0; }
    virtual void setMute(bool on) = 0;
    virtual bool isMute() const = 0;
    virtual QHash<int, int> equalizerSettings() const { return QHash<int, int>(); }
    virtual void setEqualizerSettings(const QHash<int, int> &) {}
    QString name() const { return d.name; }
protected:
    void statusChanged(int status)
    {
        Q_ASSERT(d.tail);
        QMetaObject::invokeMethod(d.tail, "statusChanged", Q_ARG(int, status));
    }
    Backend(const QString &name, QObject *tail)
    {
        Q_ASSERT(tail);
        d.name = name;
        d.tail = tail;
    }


    struct Data {
        QString name;
        QObject *tail;
    } d;
};

#endif
