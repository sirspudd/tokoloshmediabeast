Copyright (c) 2010, Anders Bakken, Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
