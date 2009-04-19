#include "config.h"
QSettings *Config::instance = 0;
QStringList Config::unused;
QStringList Config::unusedArguments()
{
    return QStringList(unused.mid(1)).filter(QRegExp("^..*$"));
}

void Config::useArg(int index)
{
    Q_ASSERT(index < unused.size());
    unused[index].clear();
}

QVariant Config::valueFromCommandLine(const QString &key)
{
    const QStringList args = QCoreApplication::arguments();
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
    if (!instance) {
        QString fileName = valueFromCommandLine(QLatin1String("conf")).toString();
        if (!fileName.isEmpty()) {
            if (fileName == QLatin1String("none")
                || fileName == QLatin1String("null")
                || fileName == QLatin1String("/dev/null")) {
                fileName.clear();
//         } else if (!QFile::exists(fileName)) {
//             qWarning("%s doesn't seem to exist", qPrintable(fileName));
            }
            instance = new QSettings(fileName, QSettings::IniFormat);
        } else {
            instance = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                     QLatin1String("Donders"), QLatin1String("TokoloshUI"));
        }
    }
    return instance;
}

void Config::initUnused()
{
    if (unused.isEmpty()) {
        unused = QCoreApplication::arguments();
        unused.replaceInStrings(QRegExp(QLatin1String("--store"), Qt::CaseInsensitive), QString());
        unused.replaceInStrings(QRegExp(QLatin1String("--save"), Qt::CaseInsensitive), QString());
    }
}
