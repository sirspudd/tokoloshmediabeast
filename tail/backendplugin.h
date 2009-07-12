#ifndef BACKENDPLUGIN_H
#define BACKENDPLUGIN_H

#include <QStringList>
class Backend;
class BackendPlugin
{
public:
    BackendPlugin(const QStringList &list) : k(list) {}
    virtual ~BackendPlugin() {}
    QStringList keys() const { return k; }
//    virtual Backend *createBackend(QObject *parent) = 0;
    virtual QObject *createBackend(QObject *parent) = 0;


private:
    const QStringList k;
};

typedef BackendPlugin *(*CreateBackend)();

#endif
