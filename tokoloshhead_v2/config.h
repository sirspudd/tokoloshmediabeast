#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QRegExp>
#include <QSettings>
#include <QLatin1String>
#include <QVariant>
#include <QDebug>
class Config
{
public:
    static bool isEnabled(const QString &key, bool defaultValue = false)
    {
        const QStringList args = QCoreApplication::arguments();
        enum { Unset = -1, False = 0, True = 1 } value = Unset;
        struct {
            const char *prefix;
            const char *suffix;
            const bool enable;
        } const options[] = {
            { "", "", true, },
            { "enable-?", "", true },
            { "", "=yes", true },
            { "", "=1", true },
            { "", "=true", true },
            { "no-?", "", false },
            { "disable-?", "", false },
            { "", "=no", false },
            { "", "=0", false },
            { "", "=false", false },
            { 0, 0, false }
        };
        QRegExp rx;
        rx.setCaseSensitivity(Qt::CaseInsensitive);

        for (int i=0; options[i].prefix; ++i) {
            rx.setPattern(QString("--?%1%2%3").
                          arg(options[i].prefix).
                          arg(key).
                          arg(options[i].suffix));
            const int arg = args.indexOf(rx);
            if (arg != -1) {
                useArg(arg);
                value = options[i].enable ? True : False;
                break;
            }
        }

        if (value == Unset) {
            QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                               QLatin1String("Donders"), QLatin1String("TokoloshUI"));
            if (settings.contains(key.toLower())) {
                value = settings.value(key.toLower()).toBool() ? True : False;
            }
        } else if (args.contains(QLatin1String("--store"), Qt::CaseInsensitive)
                   || args.contains(QLatin1String("--save"), Qt::CaseInsensitive)) {
            Config::setValue(key.toLower(), (value == True));
        }
        return value == Unset ? defaultValue : (value == True);
    }

    template <typename T> static T value(const QString &key, const T &defaultValue = T(), bool *ok = 0)
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

        if (value.isNull()) {
            value = QSettings(QSettings::IniFormat, QSettings::UserScope,
                              QLatin1String("Donders"), QLatin1String("TokoloshUI"))
                    .value(key.toLower());
        } else if (args.contains(QLatin1String("--store"), Qt::CaseInsensitive)
                   || args.contains(QLatin1String("--save"), Qt::CaseInsensitive)) {
            Config::setValue(key.toLower(), qVariantValue<T>(value));
        }

        if (value.isNull()) {
            if (ok)
                *ok = false;
            return defaultValue;
        }
        if (ok)
            *ok = true;

        if (qVariantCanConvert<T>(value)) {
            return qVariantValue<T>(value);
        }

        QString txt;
        QDebug(&txt) << value;

        qWarning("Can't convert %s to %s",
                 qPrintable(txt), qVariantFromValue<T>(T()).typeName());
        return defaultValue;
    }

    template <typename T> static void setValue(const QString &key, const T &t)
    {
        QSettings(QSettings::IniFormat, QSettings::UserScope,
                  QLatin1String("Donders"), QLatin1String("TokoloshUI"))
            .setValue(key, qVariantValue<T>(t));
    }

    static QStringList unusedArguments();
private:
    Config() {}
    static void useArg(int index);
    static QStringList unused;
};

#endif
