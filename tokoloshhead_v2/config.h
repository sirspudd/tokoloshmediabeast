#ifndef CONFIG_H
#define CONFIG_H

#include <QtCore>
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
            Settings settings;
            if (settings->contains(key.toLower())) {
                value = settings->value(key.toLower()).toBool() ? True : False;
            }
        } else if (args.contains(QLatin1String("--store"), Qt::CaseInsensitive)
                   || args.contains(QLatin1String("--save"), Qt::CaseInsensitive)) {
            Config::setValue(key.toLower(), (value == True));
        }
        return value == Unset ? defaultValue : (value == True);
    }

    template <typename T> static T value(const QString &key, const T &defaultValue = T(), bool *ok = 0)
    {
        QVariant value = valueFromCommandLine(key);

        if (value.isNull()) {
            value = Settings()->value(key.toLower());
        } else {
            const QStringList args = QCoreApplication::arguments();
            if (args.contains(QLatin1String("--store"), Qt::CaseInsensitive)
                || args.contains(QLatin1String("--save"), Qt::CaseInsensitive)) {
                Config::setValue(key.toLower(), qVariantValue<T>(value));
            }
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
        Settings()->setValue(key, qVariantValue<T>(t));
    }

    static QStringList unusedArguments();
private:
    struct Settings {
        Settings();
        ~Settings();
        inline const QSettings *operator->() const { return settings; }
        inline QSettings *operator->() { return settings; }
        QSettings *settings;
    };
    Config() {}
    static QVariant valueFromCommandLine(const QString &key);
    static void useArg(int index);
    static QStringList unused;
};

#endif
