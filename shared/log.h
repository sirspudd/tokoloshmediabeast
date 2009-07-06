#ifndef LOG_H
#define LOG_H

#include <QtCore>
#include "mutex.h"

class Log
{
public:
    static QDebug log(int verbosity = -1);
    static QString logFile();
    static bool setLogFile(const QString &file);
    static void setLogDevice(QIODevice *device);
private:
    Log() {}
    static QIODevice *logDevice;
    static Mutex logDeviceMutex;
};


#endif
