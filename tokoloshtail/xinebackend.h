#ifndef XINEBACKEND_H
#define XINEBACKEND_H

#include <QtCore>
#include "backend.h"

struct Private;
class XineBackend : public Backend
{
    Q_OBJECT
public:
    XineBackend();
    virtual ~XineBackend();
    virtual bool initBackend();
    virtual void shutdown();
    virtual bool trackData(TrackData *data, const QString &path, uint types = All) const;
    virtual bool isValid(const QString &fileName) const;
    virtual void play();
    virtual void pause();
    virtual void setProgress(int type, int progress);
    virtual int progress(int type);
    virtual void stop();
    virtual bool loadFile(const QString &fileName);
    virtual int status() const;
    virtual int volume() const;
    virtual void setVolume(int vol);
    virtual QString errorMessage() const;
    virtual int errorCode() const;
    virtual void setMute(bool on);
    virtual bool isMute() const;
    virtual int flags() const;
    virtual QByteArray equalizerSettings() const;
    virtual void setEqualizerSettings(const QByteArray &data);
private:
    Private *d;
};

#endif
