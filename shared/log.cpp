#include "log.h"

#include "config.h"

class DevNull : public QIODevice
{
public:
    static DevNull *instance()
    {
        static Mutex mutex;
        MutexLocker locker(&mutex);
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
};

QIODevice *Log::logDevice = 0;
Mutex Log::logDeviceMutex;

QDebug Log::log(int verbosity)
{
    MutexLocker locker(&logDeviceMutex);
    enum { DefaultVerbosity = 0 };
    if (verbosity <= Config::value<int>("verbosity", DefaultVerbosity)) {
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
    MutexLocker locker(&logDeviceMutex);
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
    MutexLocker locker(&logDeviceMutex);
    delete logDevice;
    logDevice = device;
}
