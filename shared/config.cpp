#include "config.h"

QSettings *Config::instance = 0;
QStringList Config::unused;
QStringList Config::args;
Mutex Config::mutex;

// ### This class is not thread safe yet ###

QStringList Config::unusedArgumentsImpl()
{
    initUnused();
//    MutexLocker locker(&argsMutex);
    return QStringList(unused.mid(1)).filter(QRegExp("^..*$"));
}

void Config::useArg(int index)
{
    initUnused();
//    MutexLocker locker(&argsMutex);
    Q_ASSERT(index < unused.size());
    unused[index].clear();
}

QVariant Config::valueFromCommandLine(const QString &key)
{
    const QStringList args = Config::argumentsImpl();
    QRegExp rx(QString("--?%1=(.*)").arg(key));
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    QVariant value;
    int arg = args.indexOf(rx);
    if (arg != -1) {
        value = rx.cap(1);
        useArg(arg);
    } else {
        rx.setPattern(QString("--?%1$").arg(key));
        arg = args.indexOf(rx);
        if (arg != -1 && arg + 1 < args.size()) {
            useArg(arg);
            useArg(arg + 1);
            value = args.value(arg + 1);
        }
    }
    return value;
}

QSettings * Config::settings()
{
//    static Mutex settingsMutex;
//    MutexLocker locker(&settingsMutex);
    if (!instance) {
        QString fileName = valueFromCommandLine("conf").toString();
        if (!fileName.isEmpty()) {
            if (fileName == "none"
                || fileName == "null"
                || fileName == "/dev/null") {
                fileName.clear();
//         } else if (!QFile::exists(fileName)) {
//             qWarning("%s doesn't seem to exist", qPrintable(fileName));
            }
            instance = new QSettings(fileName, QSettings::IniFormat);
        } else {
            instance = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                     QCoreApplication::organizationName(), QCoreApplication::applicationName());
        }
    }
    return instance;
}

void Config::initUnused()
{
//    MutexLocker locker(&unusedMutex);
    if (unused.isEmpty()) {
        unused = Config::argumentsImpl();
        unused.replaceInStrings(QRegExp("--store", Qt::CaseInsensitive), QString());
        unused.replaceInStrings(QRegExp("--save", Qt::CaseInsensitive), QString());
    }
}

bool Config::store()
{
    static enum { DontStore = 0x0, Store = 0x1, Unset = 0x2 } state = Unset;
    if (state == Unset) {
        const QStringList args = Config::argumentsImpl();
        state = (args.contains("--store", Qt::CaseInsensitive)
                 || args.contains("--save", Qt::CaseInsensitive)
                 ? Store
                 : DontStore);
    }

    return (state == Store);
}

QStringList Config::argumentsImpl()
{
    if (args.isEmpty())
        args = QCoreApplication::arguments();
    return args;
}

void Config::init(int argc, char **argv)
{
    MutexLocker locker(&mutex);
    args.clear();
    for (int i=0; i<argc; ++i) {
        args.append(QString::fromLocal8Bit(argv[i]));
    }
    (void)settings();
}
