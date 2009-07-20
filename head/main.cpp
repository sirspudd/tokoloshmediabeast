#include <QtGui>
#include <QtDBus>
#include "player.h"
#include "config.h"
#include "log.h"
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
        return var.toStringList().join("\n");
    } else {
        return var.toString();
    }
}

static inline QString toString(const QList<int> &args)
{
    QStringList list;
    foreach(int arg, args) {
        list.append(QMetaType::typeName(arg));
    }
    return list.join(", ");
}

int main(int argc, char *argv[])
{
    ::initApp("tokoloshhead", argc, argv);
    const bool gui = startGui();
    if (gui) {
        new QApplication(argc, argv);
    } else {
        new QCoreApplication(argc, argv);
    }
    bool ret; // for QApplication::exec() {
    QObject interfaceManager; // so the interface gets deleted
    QDBusInterface *interface = 0;
    if (Config::isEnabled("dbus", true)) {
        if (!QDBusConnection::sessionBus().isConnected()) {
            fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                    "To start it, run:\n"
                    "\teval `dbus-launch --auto-syntax`\n");
            return 1;
        }

        interface = new QDBusInterface(SERVICE_NAME, "/", QString(), QDBusConnection::sessionBus(), &interfaceManager);
        if (!interface->isValid() || Config::isEnabled("restartbackend")) { // tokoloshtail will kill existing process
            if (!QProcess::startDetached("tokoloshtail")
                && !QProcess::startDetached(QCoreApplication::applicationDirPath() + "/../bin/tokoloshtail")
                && !interface->isValid()) {
                qWarning("Can't start tokoloshtail");
                return 1;
            }
        }

        int max = 10;
        while (!interface->isValid() && max--) {
            delete interface;
            interface = new QDBusInterface(SERVICE_NAME, "/", QString(), QDBusConnection::sessionBus(), &interfaceManager);
            // ### nasty way of doing it
            Log::log(10) << "Waiting for tokoloshtail...";
            ::sleep(1000);
        }

        if (!interface->isValid()) {
            qWarning("Can't connect to backend");
            return 1;
        }
        interface->call("setCWD", QDir::currentPath());
        if (argc > 1) {
            const QStringList cmdLineArgs = Config::arguments();
            const int argCount = cmdLineArgs.size();
            for (int i=1; i<argCount; ++i) {
                const QString &arg = cmdLineArgs.at(i);
                if (arg == "--list-methods") {
                    const QMetaObject *metaObject = interface->metaObject();
                    for (int j=metaObject->methodOffset(); j<metaObject->methodCount(); ++j) {
                        printf("%s\n", metaObject->method(j).signature());
                    }
                    return 0;
                }
                const Function function = QDBusReply<Function>(interface->call("findFunction", arg)).value();
                if (!function.name.isEmpty()) {
                    Log::log(10) << arg << function.name << function.args;
                    QString error;
                    foreach(const QList<int> &functionArgs, function.args) {
                        bool foundError = false;
//                        qDebug() << function.name << ::toString(functionArgs);
                        if (argCount - i - 1 < functionArgs.size()) {
                            error = QString("Not enough arguments specified for %1 needed %2 (%3), got %4").
                                    arg(function.name).arg(functionArgs.size()).arg(::toString(functionArgs)).arg(argCount - i - 1);
                            continue;
                        }

                        QList<QVariant> arguments;
                        int ii = i;
                        for (int j=0; j<functionArgs.size(); ++j) {
                            QVariant variant = cmdLineArgs.at(++ii);
                            if (!variant.convert(static_cast<QVariant::Type>(functionArgs.at(j)))) {
                                error = QString("Can't convert %1 to %2").arg(cmdLineArgs.at(ii)).arg(QMetaType::typeName(functionArgs.at(j)));
                                foundError = true;
                                break;
                            }
                            arguments.append(variant);
                        }
                        if (foundError) {
                            continue;
                        }
                        error.clear();
                        i = ii;
                        // should this be async?
                        Log::log() << "Calling" << function.name << arguments;
                        const QDBusMessage ret = interface->callWithArgumentList(QDBus::Block, function.name, arguments);
                        // ### what if it can't call the function?
                        if (!ret.arguments().isEmpty())
                            printf("%s\n", qPrintable(toString(ret.arguments().first())));
                        break;
                    }
                    if (!error.isEmpty()) {
                        qWarning("Error: %s", qPrintable(error));
                        return 1;
                    }
                    return 0;
                } else if (!QFile::exists(arg)) {
                    const QString lastError = QDBusReply<QString>(interface->call("lastError")).value();
                    qWarning("Error: %s", qPrintable(lastError));
                    return 1;
                }
            }
        }

        foreach(const QString &arg, Config::unusedArguments()) {
            const QFileInfo fi(arg);
            if (!fi.exists()) {
                if (!arg.startsWith("-")) {
                    qWarning("%s doesn't seem to exist", qPrintable(arg));
                    return 1;
                }
                continue;
            }
            interface->call("load", fi.absoluteFilePath()); // ### should this rely on where the
        }
        // ### handle file args
        if (!gui) {
            interface->call("sendWakeUp");
            return 0;
        }
    }

    Player player(interface);
    player.show();
    ret = qApp->exec();
    if (Config::isEnabled("pauseonexit", true)) {
        interface->call("pause");
    }
    delete qApp;
    return ret;
}
