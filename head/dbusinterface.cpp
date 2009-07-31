#include "dbusinterface.h"
#include <QtDBus>
#include "log.h"

class DBusInterfacePrivate : public QObject
{
    Q_OBJECT
public:
    DBusInterfacePrivate()
        : QObject(QCoreApplication::instance()), interface(0)
    {
        qDeleteAll(remoteSlots);
        qDeleteAll(remoteSignals);
    }

public slots:
    void onSenderDestroyed(QObject *o)
    {
        for (int i=remoteSlots.size() - 1; i>=0; --i) {
            if (remoteSlots.at(i)->object == o)
                delete remoteSlots.takeAt(i);
        }
    }

    void onReceiverDestroyed(QObject *o)
    {
        for (int i=remoteSignals.size() - 1; i>=0; --i) {
            if (remoteSignals.at(i)->object == o)
                delete remoteSignals.takeAt(i);
        }
    }

public:
    struct Connection {
        Connection(QObject *o, const char *f, const char *t) : object(o), from(f), to(t) {}
        QObject *object;
        const char *from;
        const char *to;
    };

    bool ensureInterface(int maxWait = 10000)
    {
        if (interface && interface->isValid())
            return true;
        QTime timer;
        timer.start();
        do {
            delete interface;
            interface = new QDBusInterface(SERVICE_NAME, "/", QString(), QDBusConnection::sessionBus(), this);
            if (interface->isValid()) {
                foreach(const Connection *sig, remoteSignals) {
                    interface->connection().connect(SERVICE_NAME, "/", QString(), sig->from, sig->object, sig->to);
                }

                foreach(const Connection *sig, remoteSlots) {
                    connect(sig->object, sig->from, interface, sig->to);
                }
                return true;
            }
        } while (timer.elapsed() < maxWait);
        return false;
    }


    QList<Connection*> remoteSignals, remoteSlots;
    QDBusInterface *interface;
};

#include "dbusinterface.moc"

DBusInterfacePrivate *DBusInterface::instance = 0;

bool DBusInterface::connectToRemoteSignal(const char *sig, QObject *receiver, const char *member)
{
    if (!init()) {
        Log::log(0) << "DBus Interface error" << __FUNCTION__;
        return false;
    }

    if (instance->interface->connection().connect(SERVICE_NAME, "/", QString(), sig, receiver, member)) {
        instance->remoteSignals.append(new DBusInterfacePrivate::Connection(receiver, sig, member));
        return true;
    }
    Log::log(0) << "Can't make connection to remote slot" << sig << receiver << member;
    return false;
}

bool DBusInterface::connectToRemoteSlot(QObject *sender, const char *sig, const char *remoteMember)
{
    if (!init()) {
        Log::log(0) << "DBus Interface error" << __FUNCTION__;
        return false;
    }
    if (QObject::connect(sender, SIGNAL(destroyed(QObject*)), instance, SLOT(onSenderDestroyed(QObject*)))) {
        instance->remoteSlots.append(new DBusInterfacePrivate::Connection(sender, sig, remoteMember));
        return true;
    }
    Log::log(0) << "Can't make connection to remote slot" << sender << sig << remoteMember;
    return false;
}

QDBusMessage DBusInterface::callWithArgumentList(QDBus::CallMode mode,
                                                 const QString &method,
                                                 const QList<QVariant> &args)
{
    if (!init()) {
        Log::log(0) << "DBus Interface error" << __FUNCTION__;
        return QDBusMessage();
    }
    return instance->interface->callWithArgumentList(mode, method, args);
}

bool DBusInterface::callWithCallback(const QString &method,
                                     const QList<QVariant> &args,
                                     QObject *receiver, const char *member)
{
    if (!init()) {
        Log::log(0) << "DBus Interface error" << __FUNCTION__;
        return false;
    }
    return instance->interface->callWithCallback(method, args, receiver, member);
}

bool DBusInterface::init()
{
    if (!instance)
        instance = new DBusInterfacePrivate;
    return instance->ensureInterface();
}

//  LocalWords:  include
