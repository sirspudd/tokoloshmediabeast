#include <QtGui>
#include <QtDBus>
#include "player.h"
#include "config.h"
#include "../shared/global.h"

static inline bool startGui()
{
#ifdef Q_WS_X11
    if (!getenv("DISPLAY"))
        return false;
#endif
    if (Config::isEnabled("singlegui", true)) {
        if (QDir("/proc").exists()) {
            const qint64 pid = Config::value<qint64>("guiPid", -1);
            if (pid != -1 && QFile::exists(QString("/proc/%1").arg(pid)))
                return false;
            Config::setValue<qint64>("guiPid", QCoreApplication::applicationPid());
        } else {
            qWarning("singlegui stuff doesn't work on this platform. There must be some API that checks if a process is running");
        }
    }
    return true;
}

static inline QString toString(const QVariant &var)
{
    if (var.type() == QVariant::StringList) {
        return var.toStringList().join(QLatin1String("\n"));
    } else {
        return var.toString();
    }
}

static inline QMetaMethod findMethod(const QMetaObject *metaObject, const QString &arg)
{
    QMetaMethod best;

    const int methodCount = metaObject->methodCount();
    for (int i=metaObject->methodOffset(); i<methodCount; ++i) {
        const QMetaMethod method = metaObject->method(i);
        if (method.methodType() != QMetaMethod::Slot
            && method.methodType() != QMetaMethod::Method)
            continue;

        const QString methodName = QString::fromLatin1(method.signature());
        if (!methodName.startsWith(arg))
            continue;

        if (methodName.size() != arg.size()) {
            // if the argument part is not specified that's still an exact match
            const int index = methodName.lastIndexOf("(");
            if (arg.size() < index) {
                if (best.signature()) {
                    qWarning("Ambigious request. Could match either %s or %s", best.signature(), method.signature());
                    // could maybe match more as well, should we print that as well?
                    return QMetaMethod();
                }
                best = method;
                continue;
            }
        }
        best = method;
        break;
    }
    return best;
}

int main(int argc, char *argv[])
{
    QCoreApplication *coreApp = new QCoreApplication(argc, argv);
    ::initApp(coreApp, "tokoloshhead");
    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
        return 1;
    }

    QDBusInterface *iface = new QDBusInterface(SERVICE_NAME, "/", "", QDBusConnection::sessionBus());
    if (!iface->isValid() || Config::isEnabled("restartbackend")) { // tokoloshtail will kill existing process
        if (!QProcess::startDetached("tokoloshtail")) {
            QProcess::startDetached("../tokoloshtail/tokoloshtail");
        }
    }

    int max = 10;
    while (!iface->isValid() && max--) {
        delete iface;
        iface = new QDBusInterface(SERVICE_NAME, "/", "", QDBusConnection::sessionBus());
        // ### nasty way of doing it
        printf("Waiting for tokoloshtail...\n");
        ::sleep(1000);
    }

    if (argc > 1) {
        iface->call("setCWD", QDir::currentPath());
        const QStringList args = Config::arguments();
        for (int i=1; i<argc; ++i) {
            const QString &arg = args.at(i);
            if (arg == "--list-methods") {
                const QMetaObject *dbusInterfaceMetaObject = iface->metaObject();
                for (int j=dbusInterfaceMetaObject->methodOffset(); j<dbusInterfaceMetaObject->methodCount(); ++j) {
                    printf("%s\n", dbusInterfaceMetaObject->method(j).signature());
                }
                delete iface;
                return 0;
            }
            const QMetaMethod method = findMethod(iface->metaObject(), arg);
            if (!method.signature())
                continue;

            const QList<QByteArray> parameterTypes = method.parameterTypes();
            if (argc - i - 1 < parameterTypes.size()) {
                qWarning("Not enough arguments specified for %s needed %d, got %d",
                         method.signature(), parameterTypes.size(), argc - i - 1);
                return 1; // ### ???
            }
            QList<QVariant> arguments;
            for (int j=0; j<parameterTypes.size(); ++j) {
                const int type = QMetaType::type(parameterTypes.at(j).constData());
                QVariant variant = args.at(++i);
                if (!variant.convert(static_cast<QVariant::Type>(type))) {
                    qWarning("Can't convert %s to %s", qPrintable(args.at(i)), parameterTypes.at(i).constData());
                    return 1; // ### ???
                }
                arguments.append(variant);
            }
            QString methodName = QString::fromLatin1(method.signature());
            methodName.chop(methodName.size() - methodName.indexOf('('));
            const QDBusMessage ret = iface->callWithArgumentList(QDBus::Block, methodName, arguments);
            if (!ret.arguments().isEmpty())
                printf("%s\n", qPrintable(toString(ret.arguments().first())));
            return 0;
        }
    }


    foreach(QString arg, Config::unusedArguments()) {
        const QFileInfo fi(arg);
        if (!fi.exists()) {
            if (!arg.startsWith("-")) {
                qWarning("%s doesn't seem to exist", qPrintable(arg));
                return 1;
            }
            continue;
        }
//        dbusInterface.load(arg);
    }
    // ### handle file args
    if (!startGui()) {
        iface->call("sendWakeUp");
        return 0;
    }

    delete coreApp;

    QApplication app(argc, argv);
    ::initApp(&app, "tokoloshhead");
    Player player(iface);
//     const QDBusPendingReply<int> vol = dbusInterface.volume();
//     if (dbusInterface.lastError().type() != QDBusError::NoError) {
//         if (!QProcess::startDetached("tokoloshtail")) {
//             qWarning("Can't start tokoloshtail");
//         }
//     }
//    qDebug() << dbusInterface.lastError() << dbusInterface.lastError().type();
    if (!player.setSkin(Config::value<QString>("skin", QString(":/skins/dullSod")))) {
        const bool ret = player.setSkin(QLatin1String(":/skins/dullSod"));
        Q_ASSERT(ret);
        Q_UNUSED(ret);
    }
    player.show();
    const bool ret = app.exec();
    delete iface;
    return ret;
}
