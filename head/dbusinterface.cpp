/*
    Copyright (c) 2010 Anders Bakken
    Copyright (c) 2010 Donald Carr
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer. Redistributions in binary
    form must reproduce the above copyright notice, this list of conditions and
    the following disclaimer in the documentation and/or other materials
    provided with the distribution. Neither the name of any associated
    organizations nor the names of its contributors may be used to endorse or
    promote products derived from this software without specific prior written
    permission. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
    NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
    OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

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
        } while (maxWait < 0 || timer.elapsed() < maxWait);
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
