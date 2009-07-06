#include "log.h"
#include "config.h"

class DevNull : public QIODevice
{
public:
    static DevNull *instance()
    {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        static DevNull *instance = new DevNull;
        return instance;
    }
    virtual qint64 readData(char*, qint64) { return -1; }
    virtual qint64 writeData(const char*, qint64) { return -1; }
private:
    DevNull()
        : QIODevice(QCoreApplication::instance())
    {}
};

QIODevice *Log::Private::logDevice = 0;
QMutex Log::Private::logDeviceMutex;

QDebug Log::log(int verbosity)
{
    QMutexLocker locker(&Private::logDeviceMutex);
    if (verbosity < Config::value<int>("verbosity", INT_MAX)) {
        if (!Private::logDevice) {
            return qDebug();
        } else {
            return QDebug(Private::logDevice);
        }
    } else {
        return QDebug(DevNull::instance());
    }
}

QString Log::logFile()
{
    QMutexLocker locker(&Private::logDeviceMutex);
    if (!Private::logDevice) {
        return "stderr";
    } else if (QFile *file = qobject_cast<QFile*>(Private::logDevice)) {
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
    QMutexLocker locker(&Private::logDeviceMutex);
    delete Private::logDevice;
    Private::logDevice = device;
}
