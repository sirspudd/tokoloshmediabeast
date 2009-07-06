#include "../shared/global.h"
#include <QtCore>
#include <QtDBus>
#include "backendplugin.h"
//#undef PLUGINDIR

int main(int argc, char *argv[])
{
    ::initApp(QLatin1String("tokoloshtail"), argc, argv);
    QCoreApplication app(argc, argv);
    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
        return 1;
    }

//    const QString pluginDirectory = Config::value<QString>("plugindir", PLUGINDIR); // ### Can't make this work
    const QString pluginDirectory = Config::value<QString>("plugindir", QCoreApplication::applicationDirPath() + "/../tokoloshtail/plugins");
    const QString backendName = Config::value<QString>(QLatin1String("backend"), QLatin1String("xine"));
    QDir dir(pluginDirectory);
    if (!dir.exists()) {
        qWarning("'%s' doesn't seem to exist", qPrintable(pluginDirectory));
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
                    qWarning("%s doesn't seem to be able to create a backend", qPrintable(fi.absoluteFilePath()));
                }
                delete interface;
            } else if (!interface) {
                delete lib;
            } else {
                candidates[lib] = interface;
            }
        } else {
            if (lib->isLoaded())
                qDebug() << lib->errorString();
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
        qWarning("Can't find a suitable backend");
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
        qWarning("Can't seem to register service %s", qPrintable(QDBusConnection::sessionBus().lastError().message()));
        return 1;
    }

    bool ret;
    bool ok = backend->metaObject()->invokeMethod(backend, "initBackend", Q_RETURN_ARG(bool, ret));
    if (!ret || !ok) {
        int errorCode;
        QString errorMessage;
        ok = backend->metaObject()->invokeMethod(backend, "errorCode", Q_RETURN_ARG(int, errorCode));
        ok = backend->metaObject()->invokeMethod(backend, "errorMessage", Q_RETURN_ARG(QString, errorMessage));
        qWarning("%s:%d", qPrintable(errorMessage), errorCode);
        return 1;
    }
    QDBusConnection::sessionBus().registerObject("/", backend, QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals);
    printf("Using %s\n", backend->metaObject()->className());

    const int appReturnValue = app.exec();
    delete backend;
    library->unload();
    delete library;
    return appReturnValue;

}
