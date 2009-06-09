#include "xinebackend.h"
#include "../shared/global.h"
#include <QtCore>
#include <QtDBus>
#ifdef Q_WS_WIN
#include <windows.h>
#else
#include <unistd.h>
#endif
void sleep(int msec)
{
#ifdef Q_WS_WIN
    Sleep(msec);
#else
    usleep(1000*msec);
#endif
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    ::initApp(&app, QLatin1String("tokoloshtail"));
    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
        return 1;
    }

    {
        QDBusInterface iface(SERVICE_NAME, "/", "", QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.call("quit");
        }
    }

    bool registered = false;
    for (int i=0; i<5; ++i) {
        if (QDBusConnection::sessionBus().registerService(SERVICE_NAME)) {\
            registered = true;
            break;
        }
        sleep(500);
    }
    if (!registered) {
        qWarning("Can't seem to register service %s", qPrintable(QDBusConnection::sessionBus().lastError().message()));
        return 1;
    }

    XineBackend daemon;
    if (!daemon.initBackend()) {
        qWarning() << daemon.errorCode() << daemon.errorMessage();
        return 1;
    }
    QDBusConnection::sessionBus().registerObject("/", &daemon, QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals);

    return app.exec();
}
