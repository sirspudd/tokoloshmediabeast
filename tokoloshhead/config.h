#ifndef CONFIG_H
#define CONFIG_H

#include <QtCore>
class Config
{
public:
    static void setEnabled(const QString &key, bool on)
    {
        Config::setValue(key, on);
    }

    static bool isEnabled(const QString &k, bool defaultValue = false)
    {
        const QString key = k.toLower();
        const QStringList args = Config::arguments();
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

            QSettings *s = settings();
            if (s->contains(key)) {
                value = s->value(key).toBool() ? True : False;
            }
        } else if (store()) {
            Config::setValue(key, (value == True));
        }
        return value == Unset ? defaultValue : (value == True);
    }

    template <typename T> static bool contains(const QString &key)
    {
        arguments();
        bool ok;
        (void)value<T>(key, T(), &ok);
        return ok;
    }

    template <typename T> static T value(const QString &k, const T &defaultValue = T(), bool *ok = 0)
    {
        arguments();
        const QString key = k.toLower();
        QVariant value = valueFromCommandLine(key);

        if (value.isNull()) {
            value = settings()->value(key);
        } else if (store()) {
            Config::setValue(key, qVariantValue<T>(value));
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
        arguments();
        QSettings *s = settings();
        s->setValue(key.toLower(), qVariantValue<T>(t));
        s->sync();
    }

    static QStringList unusedArguments();
    static QStringList arguments();
private:
    static QSettings *settings();
    static void initUnused();
    static bool store();
    Config() {}
    static QVariant valueFromCommandLine(const QString &key);
    static void useArg(int index);
    static QStringList unused;
    static QSettings *instance;
};

#endif
