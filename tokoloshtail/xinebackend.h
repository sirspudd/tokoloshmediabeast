#ifndef XINEBACKEND_H
#define XINEBACKEND_H

#include <QtCore>
#include "backend.h"
#include "backendplugin.h"

struct Private;
class Q_DECL_EXPORT XineBackend : public Backend
{
    Q_OBJECT
public:
    XineBackend(QObject *parent = 0);
    virtual ~XineBackend();
    virtual bool initBackend();
    virtual void shutdown();
    virtual bool trackData(TrackData *data, const QString &path, int types = All) const;
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
    virtual QHash<int, int> equalizerSettings() const;
    virtual void setEqualizerSettings(const QHash<int, int> &eq);
private:
    Private *d;
};

class Q_DECL_EXPORT XineBackendPlugin : public BackendPlugin
{
public:
    XineBackendPlugin() : BackendPlugin(QStringList() << QLatin1String("xine") << QLatin1String("xinebackend")) {}
    virtual QObject *createBackend(QObject *parent)
    {
        return new XineBackend(parent);
    }

};
#endif
