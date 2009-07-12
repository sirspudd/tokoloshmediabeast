#include "../shared/global.h"
#include <QtCore>
#include <QtDBus>
#include "log.h"
#include "tail.h"
#include "backendplugin.h"
//#undef PLUGINDIR

class LibraryCleanupHandler
{
public:
    LibraryCleanupHandler()
        : library(0)
    {}

    ~LibraryCleanupHandler()
    {
        return;
        if (library) {
            library->unload();
            delete library;
        }
    }
    QLibrary *library;
};

int main(int argc, char *argv[])
{
    int ret = -1;
    LibraryCleanupHandler cleanup;
    {
        ::initApp("tokoloshtail", argc, argv);
        QCoreApplication app(argc, argv);
        if (!QDBusConnection::sessionBus().isConnected()) {
            fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                    "To start it, run:\n"
                    "\teval `dbus-launch --auto-syntax`\n");
            return 1;
        }
        {
            QDBusInterface iface(SERVICE_NAME, "/");
            if (iface.isValid()) {
                iface.call("quit");
            }
        }


//    const QString pluginDirectory = Config::value<QString>("plugindir", PLUGINDIR); // ### Can't make this work
        const QString pluginDirectory = Config::value<QString>("plugindir", QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../plugins"));
        Log::log(10) << "Using plugin directory" << pluginDirectory;
        const QString backendName = Config::value<QString>("backend", "xine");
        Log::log(10) << "Searching for backend" << backendName;
        QDir dir(pluginDirectory);
        if (!dir.exists()) {
            Log::log(0) << pluginDirectory << " doesn't seem to exist";
            return 1;
        }
        {
            Tail tail;
            Backend *backend = 0;
            QLibrary *library = 0;
            QHash<QLibrary*, BackendPlugin*> candidates;

            foreach(const QFileInfo &fi, dir.entryInfoList(QDir::Files, QDir::Size)) {
                QLibrary *lib = new QLibrary(fi.absoluteFilePath());
                CreateBackend createBackend = 0;
                if (lib->load() && (createBackend = (CreateBackend)lib->resolve("createTokoloshBackendInterface"))) {
                    BackendPlugin *interface = createBackend();
                    if (interface && interface->keys().contains(backendName, Qt::CaseInsensitive)) {
                        backend = interface->createBackend(&tail);
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
            cleanup.library = library;

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

            if (!tail.setBackend(backend)) {
                Log::log(0) << backend->errorMessage() << backend->errorCode();
                return 1;
            }
            QDBusConnection::sessionBus().registerObject("/", &tail,
                                                         QDBusConnection::ExportScriptableSlots
                                                         |QDBusConnection::ExportScriptableSignals);

            Log::log(10) << "Using" << backend->name();
            ret = app.exec();
        }
    }
    return ret;
}
