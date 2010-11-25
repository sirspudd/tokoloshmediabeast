Copyright (c) 2010, Anders Bakken, Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
