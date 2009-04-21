#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore>
#include "playlist.h"
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
        Portion // 100th of a percent
    };

    Backend() {}
    virtual ~Backend() {}

public slots:
    inline bool init() { if (status() == Uninitalized) return initBackend(); return true; }
    virtual void shutdown() = 0;
    virtual QVariant field(const QString &fileName, Playlist::Field field) const = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void setProgress(ProgressType type, int progress) = 0;
    virtual int progress(ProgressType type) = 0;
    virtual void stop() = 0;
    virtual bool load(const QString &fileName) = 0;
    virtual QString currentTrack() const = 0;
    virtual Status status() const = 0;
    virtual int volume() const = 0;
    virtual void setVolume(int vol) = 0;
    virtual bool initBackend() = 0;
    virtual QString errorMessage() const { return QString(); }
    virtual int errorCode() const { return 0; }
signals:
    void statusChanged(Status status);
};

#endif
