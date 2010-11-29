Copyright (c) 2010 Anders Bakken
Copyright (c) 2010 Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "config.h"

QSettings *Config::instance = 0;
QStringList Config::unused;
QStringList Config::args;

// ### This class is not thread safe yet ###

QStringList Config::unusedArguments()
{
    initUnused();
    return QStringList(unused.mid(1)).filter(QRegExp("^..*$"));
}

void Config::useArg(int index)
{
    initUnused();
    Q_ASSERT(index < unused.size());
    unused[index].clear();
}

QVariant Config::valueFromCommandLine(const QString &key)
{
    const QStringList args = Config::arguments();
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
    if (unused.isEmpty()) {
        unused = Config::arguments();
        unused.replaceInStrings(QRegExp("--store", Qt::CaseInsensitive), QString());
        unused.replaceInStrings(QRegExp("--save", Qt::CaseInsensitive), QString());
    }
}

bool Config::store()
{
    static enum { DontStore = 0x0, Store = 0x1, Unset = 0x2 } state = Unset;
    if (state == Unset) {
        const QStringList args = Config::arguments();
        state = (args.contains("--store", Qt::CaseInsensitive)
                 || args.contains("--save", Qt::CaseInsensitive)
                 ? Store
                 : DontStore);
    }

    return (state == Store);
}

QStringList Config::arguments()
{
    if (args.isEmpty())
        args = QCoreApplication::arguments();
    return args;
}

void Config::init(int argc, char **argv)
{
    args.clear();
    for (int i=0; i<argc; ++i) {
        args.append(QString::fromLocal8Bit(argv[i]));
    }
    (void)settings();
}
