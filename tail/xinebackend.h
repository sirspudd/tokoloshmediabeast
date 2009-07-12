#ifndef XINEBACKEND_H
#define XINEBACKEND_H

#include <QtCore>
#include "backend.h"
#include "backendplugin.h"

struct Private;
class Q_DECL_EXPORT XineBackend : public Backend
{
public:
    XineBackend(QObject *tail);
    virtual ~XineBackend();
    virtual bool initBackend();
    virtual void shutdown();
    virtual bool trackData(TrackData *data, const QUrl &path, int types = All) const;
    virtual bool isValid(const QUrl &fileName) const;
    virtual void play();
    virtual void pause();
    virtual void setProgress(int type, int progress);
    virtual int progress(int type);
    virtual void stop();
    virtual bool loadUrl(const QUrl &fileName);
    virtual int status() const;
    virtual int volume() const;
    virtual void setVolume(int vol);
    virtual QString errorMessage() const;
    virtual int errorCode() const;
    virtual void setMute(bool on);
    virtual bool isMute() const;
    virtual int flags() const;
    virtual QHash<int, int> equalizerSettings() const;
    virtual void setEqualizerSettings(const QHash<int, int> &eq);
private:
    Private *d;
};

class Q_DECL_EXPORT XineBackendPlugin : public BackendPlugin
{
public:
    XineBackendPlugin() : BackendPlugin(QStringList() << "xine" << "xinebackend") {}
    virtual Backend *createBackend(QObject *tail)
    {
        return new XineBackend(tail);
    }

};
#endif
