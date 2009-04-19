#include "log.h"

class DevNull : public QIODevice
{
public:
    static DevNull *instance()
    {
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


QIODevice *Log::logDevice = 0;
QDebug Log::log(int verbosity)
{
    return qDebug();
//     if (!logDevice) {
//         return QDebuf

//     }
}

QString Log::logFile()
{

}
bool Log::setLogFile(const QString &file)
{

}
void Log::setLogDevice(QIODevice *device)
{

}
