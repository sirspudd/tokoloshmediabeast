#ifndef PHONONBACKEND_H
#define PHONONBACKEND_H

#include <QtCore>
#include <phonon>
#include "backend.h"
#include "backendplugin.h"

struct Private;
class Q_DECL_EXPORT PhononBackend : public Backend
{
    Q_OBJECT
public:
    PhononBackend(QObject *parent);
    virtual ~PhononBackend();
    virtual bool initBackend();
    virtual void shutdown();
    virtual bool trackData(TrackData *data, const QUrl &url, int types = All) const;
    virtual bool isValid(const QUrl &url) const;
    virtual void play();
    virtual void pause();
    virtual void setProgress(int type, int progress);
    virtual int progress(int type);
    virtual void stop();
    virtual bool loadUrl(const QUrl &url);
    virtual int status() const;
    virtual int volume() const;
    virtual void setVolume(int vol);
    virtual QString errorMessage() const;
    virtual int errorCode() const;
    virtual void setMute(bool on);
    virtual bool isMute() const;
    virtual int flags() const;
private:
    Private *d;
};

class Q_DECL_EXPORT PhononBackendPlugin : public BackendPlugin
{
public:
    PhononBackendPlugin() : BackendPlugin(QStringList() << "phonon" << "phononbackend") {}
    virtual QObject *createBackend(QObject *parent)
    {
        return new PhononBackend(parent);
    }

};


#endif
