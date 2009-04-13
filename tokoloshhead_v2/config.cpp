#include "config.h"

QStringList Config::unused;
QStringList Config::unusedArguments()
{
    return unused;
}

void Config::useArg(int index)
{
    if (unused.isEmpty()) {
        unused = QCoreApplication::arguments();
        unused.replaceInStrings(QRegExp(QLatin1String("--store"), Qt::CaseInsensitive), QString());
        unused.replaceInStrings(QRegExp(QLatin1String("--save"), Qt::CaseInsensitive), QString());
    }
    Q_ASSERT(index < unused.size());
    unused[index].clear();
}
