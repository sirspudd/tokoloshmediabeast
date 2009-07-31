#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>
#include <QDBusMessage>
#include <QString>
#include <QVariant>
#include <QList>
#include <QDBusConnection>
#include <QCoreApplication>
#include <QTime>
#include <QDBusInterface>
#include "global.h"

class DBusInterfacePrivate;
class DBusInterface
{
public:
    static bool connectToRemoteSignal(const char *sig, QObject *receiver, const char *member);
    static bool connectToRemoteSlot(QObject *sender, const char *sig, const char *remoteMember);
    static QDBusMessage call(QDBus::CallMode mode, const QString &method, const QVariant &arg = QVariant())
    {
        QList<QVariant> args;
        if (!arg.isNull())
            args.append(arg);
        return callWithArgumentList(mode, method, args);
    }
    static QDBusMessage callWithArgumentList(QDBus::CallMode mode,
                                             const QString &method,
                                             const QList<QVariant> &args);
    static bool callWithCallback(const QString &method,
                                 const QList<QVariant> &args,
                                 QObject *receiver, const char *member);
private:
    static bool init();
    static DBusInterfacePrivate *instance;
};

#endif
