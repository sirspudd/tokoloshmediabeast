#ifndef XINEBACKEND_H
#define XINEBACKEND_H

#include <QtCore>
#include "backend.h"

struct Private;
class XineBackend : public Backend
{
    Q_OBJECT
public:
    virtual ~XineBackend();
    virtual bool initBackend();
    virtual void shutdown();
    virtual QVariant field(const QString &fileName, Playlist::Field field) const;
    virtual void play();
    virtual void pause();
    virtual void setProgress(ProgressType type, int progress);
    virtual int progress(ProgressType type);
    virtual void stop();
    virtual bool load(const QString &fileName);
    virtual QString currentTrack() const;
    virtual Status status() const;
    virtual int volume() const;
    virtual void setVolume(int vol);
    virtual QString errorMessage() const;
    virtual int errorCode() const;
    virtual void setMute(bool on);
    virtual bool isMute() const;
    virtual uint flags() const;
    virtual QHash<int, int> equalizerSettings() const;
    virtual void setEqualizerSettings(const QHash<int, int> &eq);


    static XineBackend *instance();
private:
    Private *d;
    XineBackend();
    static XineBackend *inst;
};

#endif
