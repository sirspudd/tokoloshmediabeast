#include "../shared/global.h"
#include <QtCore>
#include <QtDBus>
#include "log.h"
#include "backendplugin.h"
//#undef PLUGINDIR

int main(int argc, char *argv[])
{
    ::initApp("tokoloshtail", argc, argv);
    QCoreApplication app(argc, argv);
    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
        return 1;
    }

//    const QString pluginDirectory = Config::value<QString>("plugindir", PLUGINDIR); // ### Can't make this work
    const QString pluginDirectory = Config::value<QString>("plugindir", QCoreApplication::applicationDirPath() + "/../tokoloshtail/plugins");
    Log::log(10) << "Using plugin directory" << pluginDirectory;
    const QString backendName = Config::value<QString>("backend", "xine");
    Log::log(10) << "Searching for backend" << backendName;
    QDir dir(pluginDirectory);
    if (!dir.exists()) {
        Log::log(0) << pluginDirectory << " doesn't seem to exist";
        return 1;
    }
    QObject *backend = 0;
    QLibrary *library = 0;
    QHash<QLibrary*, BackendPlugin*> candidates;

    foreach(const QFileInfo &fi, dir.entryInfoList(QDir::Files, QDir::Size)) {
        QLibrary *lib = new QLibrary(fi.absoluteFilePath());
        CreateBackend createBackend = 0;
        if (lib->load() && (createBackend = (CreateBackend)lib->resolve("createTokoloshBackendInterface"))) {
            BackendPlugin *interface = createBackend();
            if (interface && interface->keys().contains(backendName, Qt::CaseInsensitive)) {
                backend = interface->createBackend(&app);
                if (backend) {
                    library = lib;
                    break;
                } else {
                    Log::log(0) << fi.absoluteFilePath() << "doesn't seem to be able to create a backend";
                }
                delete interface;
            } else if (!interface) {
                delete lib;
            } else {
                candidates[lib] = interface;
            }
        } else {
            if (lib->isLoaded()) {
                Log::log(1) << "Can't load" << fi.absoluteFilePath() << lib->errorString();
            }
            delete lib;
        }
    }

    Q_ASSERT(!backend == !library);
    if (!backend) {
        for (QHash<QLibrary*, BackendPlugin*>::const_iterator it = candidates.begin(); it != candidates.end(); ++it) {
            const bool hadBackend = backend != 0;
            if (!backend)
                backend = it.value()->createBackend(&app);
            if (hadBackend || !backend) {
                it.key()->unload();
                delete it.key();
            } else {
                library = it.key();
            }
            delete it.value();
        }
    }
    if (!backend) {
        Log::log(0) << "Can't find a suitable backend";
        return 1;
    }
    {
        QDBusInterface iface(SERVICE_NAME, "/");
        if (iface.isValid()) {
            iface.call("quit");
        }
    }

    bool registered = false;
    for (int i=0; i<5; ++i) {
        if (QDBusConnection::sessionBus().registerService(SERVICE_NAME)) {
            registered = true;
            break;
        }
        ::sleep(500);
    }
    if (!registered) {
        Log::log(0) << "Can't seem to register service" << QDBusConnection::sessionBus().lastError().message();
        return 1;
    }

    bool ret;
    bool ok = backend->metaObject()->invokeMethod(backend, "initBackend", Q_RETURN_ARG(bool, ret));
    if (!ret || !ok) {
        int errorCode;
        QString errorMessage;
        ok = backend->metaObject()->invokeMethod(backend, "errorCode", Q_RETURN_ARG(int, errorCode));
        ok = backend->metaObject()->invokeMethod(backend, "errorMessage", Q_RETURN_ARG(QString, errorMessage));
        Log::log(0) << errorMessage << errorCode;
        return 1;
    }
    QDBusConnection::sessionBus().registerObject("/", backend, QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals);

    Log::log(10) << "Using" << backend->metaObject()->className();
    const int appReturnValue = app.exec();
    backend->metaObject()->invokeMethod(backend, "shutdown");
    delete backend;
    library->unload();
    delete library;
    return appReturnValue;

}
