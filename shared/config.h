#ifndef CONFIG_H
#define CONFIG_H
#ifdef QT_GUI_LIB
#include <QtGui>
#else
#include <QtCore>
#endif

template <typename T> bool read(const QVariant &, T &)
{
    return false;
}

template <typename T> static bool read(QSettings *settings, const QString &str, T &t)
{
    const QByteArray data = settings->value(str).toByteArray();
    if (!data.isEmpty()) {
        if (data.size() != sizeof(T)) {
            qWarning("Mismatched data for %s. Expected %d bytes, got %d",
                     qPrintable(str), int(sizeof(T)), data.size());
            return false;
        }
        memcpy(reinterpret_cast<char*>(&t), data.constData(), sizeof(T));
        return true;
    }
    return false;
}

template <typename T> static void write(QSettings *settings, const QString &str, const T &t)
{
    QByteArray byteArray(sizeof(T), 0);
    memcpy(byteArray.data(), reinterpret_cast<const char*>(&t), sizeof(T));
    settings->setValue(str, byteArray);
}

#define CONFIG_TYPE(T)                                                  \
    Q_DECLARE_METATYPE(T);                                              \
    static inline bool read(const QVariant &v, T &t) {                  \
        if (qVariantCanConvert<T>(v)) {                                 \
            t = qVariantValue<T>(v);                                    \
            return true;                                                \
        }                                                               \
        return false;                                                   \
    }                                                                   \
    static inline bool read(QSettings *settings, const QString &key,    \
                            T &t) {                                     \
        const QVariant var = settings->value(key);                      \
        if (!var.isNull() && qVariantCanConvert<T>(var)) {              \
            t = qVariantValue<T>(var);                                  \
            return true;                                                \
        }                                                               \
        return false;                                                   \
    }                                                                   \
    static inline void write(QSettings *settings, const QString &key,   \
                             const T &t) {                              \
        settings->setValue(key, qVariantFromValue<T>(t));               \
    }                                                                   \

CONFIG_TYPE(bool);
CONFIG_TYPE(qint8);
CONFIG_TYPE(qint16);
CONFIG_TYPE(qint32);
CONFIG_TYPE(qint64);

CONFIG_TYPE(quint8);
CONFIG_TYPE(quint16);
CONFIG_TYPE(quint32);
CONFIG_TYPE(quint64);

CONFIG_TYPE(float);
CONFIG_TYPE(double);

CONFIG_TYPE(QChar);
CONFIG_TYPE(QString);
CONFIG_TYPE(QStringList);
CONFIG_TYPE(QByteArray);

CONFIG_TYPE(QPoint);
CONFIG_TYPE(QSize);
CONFIG_TYPE(QRect);
CONFIG_TYPE(QLine);

CONFIG_TYPE(QPointF);
CONFIG_TYPE(QSizeF);
CONFIG_TYPE(QRectF);
CONFIG_TYPE(QLineF);

typedef QList<QVariant> VariantList;
typedef QMap<QString, QVariant> StringVariantMap;
typedef QHash<QString, QVariant> StringVariantHash;

CONFIG_TYPE(VariantList);
CONFIG_TYPE(StringVariantMap);
CONFIG_TYPE(StringVariantHash);

#ifdef QT_GUI_LIB
CONFIG_TYPE(QColor);
#endif

class Config
{
public:
    static void setEnabled(const QString &key, bool on)
    {
        Config::setValue(key, on);
    }

    static inline bool isDisabled(const QString &k, bool defaultValue = false)
    {
        return !isEnabled(k, !defaultValue);
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

        QSettings *s = settings();
        if (value == Unset) {
            if (s->contains(key)) {
                value = s->value(key).toBool() ? True : False;
            }
        } else if (store()) {
            s->setValue(key, (value == True));
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

    template <typename T> static T value(const QString &k, const T &defaultValue = T(), bool *ok_in = 0)
    {
        arguments();
        const QString key = k.toLower();
        QVariant value = valueFromCommandLine(key);
        T t;
        bool ok = false;
        QSettings *s = settings();
        if (!value.isNull()) {
            ok = ::read(value, t);
            if (ok && store())
                Config::setValue<T>(key, t);
        } else {
            ok = ::read(s, k, t);
        }

        if (ok_in)
            *ok_in = ok;
        return ok ? t : defaultValue;
    }

    template <typename T> static void setValue(const QString &key, const T &t)
    {
        arguments();
        QSettings *s = settings();
        ::write(s, key, t);
        s->sync();
    }

    static QStringList unusedArguments();
    static QStringList arguments();
    static void init(int argc, char **argv);
private:
    static QSettings *settings();
    static void initUnused();
    static bool store();
    Config() {}
    static QVariant valueFromCommandLine(const QString &key);
    static void useArg(int index);
    static QStringList unused, args;
    static QSettings *instance;
    static QMutex mutex;
};

#endif
