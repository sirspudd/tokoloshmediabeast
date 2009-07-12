#ifndef LOG_H
#define LOG_H

#include <QtCore>

/* thread safe */
//#define NO_MUTEX /* not thread safe */
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
#ifndef NO_MUTEX
    static QMutex logDeviceMutex;
#endif
};


#endif
