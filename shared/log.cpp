Copyright (c) 2010, Anders Bakken, Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "log.h"
#include "config.h"

class DevNull : public QIODevice
{
public:
    static DevNull *instance()
    {
#ifndef NO_MUTEX
        static QMutex mutex;
        QMutexLocker locker(&mutex);
#endif
        static QHash<QThread*, DevNull*> instances;
        DevNull *& inst = instances[QThread::currentThread()];
        if (!inst)
            inst = new DevNull;
        return inst;
    }
    virtual qint64 readData(char*, qint64) { return -1; }
    virtual qint64 writeData(const char*, qint64) { return -1; }
private:
    DevNull()
        : QIODevice(QCoreApplication::instance())
    {}
    static QMutex mutex;
};

QIODevice *Log::logDevice = 0;
#ifndef NO_MUTEX
QMutex Log::logDeviceMutex;
#endif

QDebug Log::log(int verb)
{
    if (verb <= verbosity()) {
        if (!logDevice) {
            return qDebug();
        } else if (QThread::currentThread() == logDevice->thread()) {
            return QDebug(logDevice);
        } else {
            // ### what do I do here?
            return qDebug();
        }
    } else {
        return QDebug(DevNull::instance());
    }
}

QString Log::logFile()
{
#ifndef NO_MUTEX
    QMutexLocker locker(&logDeviceMutex);
#endif
    if (!logDevice) {
        return "stderr";
    } else if (QFile *file = qobject_cast<QFile*>(logDevice)) {
        return QFileInfo(*file).absoluteFilePath();
    } else {
        return QString();
    }
}

bool Log::setLogFile(const QString &file)
{
    QFile *f = new QFile(file);
    if (f->open(QIODevice::Append)) {
        setLogDevice(f);
        return true;
    }
    return false;
}

void Log::setLogDevice(QIODevice *device)
{
#ifndef NO_MUTEX
    QMutexLocker locker(&logDeviceMutex);
#endif
    delete logDevice;
    logDevice = device;
}

int Log::verbosity()
{
#ifndef NO_MUTEX
    QMutexLocker locker(&logDeviceMutex);
#endif
    enum { DefaultVerbosity = 0 };
    static int verbosity = -1;
    if (verbosity == -1) {
        if (Config::isEnabled("verbose")) {
            verbosity = INT_MAX;
        } else {
            verbosity = Config::value<int>("verbosity", DefaultVerbosity);
        }
    }
    return verbosity;
}
