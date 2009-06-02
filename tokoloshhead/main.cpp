#include <QApplication>
#include <QFileInfo>
#include "player.h"
#include "config.h"
#include "global.h"
#include "tokolosh_interface.h"


static inline bool startGui()
{
#ifdef Q_WS_X11
    if (!getenv("DISPLAY"))
        return false;
#endif
    if (Config::isEnabled("singlegui", true)) {
        if (QDir("/proc").exists()) {
            const qint64 pid = Config::value<qint64>("guiPid", -1);
            if (pid != -1 && QFile::exists(QString("/proc/%1").arg(pid)))
                return false;
            Config::setValue<qint64>("guiPid", QCoreApplication::applicationPid());
        } else {
            qWarning("singlegui stuff doesn't work on this platform. There must be some API that checks if a process is running");
        }
    }
    return true;
}

static inline QMetaMethod findMethod(QString arg, const QMetaObject *metaObject)
{
    // ### need to translate
//     QRegExp rx("^-*");
//     arg.remove(rx);

    QMetaMethod best;

    const int methodCount = metaObject->methodCount();
    for (int i=metaObject->methodOffset(); i<methodCount; ++i) {
        const QMetaMethod method = metaObject->method(i);
        if (method.methodType() != QMetaMethod::Slot
            && method.methodType() != QMetaMethod::Method)
            continue;

        const QString methodName = QString::fromLatin1(method.signature());
        if (!methodName.startsWith(arg))
            continue;

        if (methodName.size() != arg.size()) {
            // if the argument part is not specified that's still an exact match
            const int index = methodName.lastIndexOf("(");
            if (arg.size() < index) {
                if (best.signature()) {
                    qWarning("Ambigious request. Could match either %s or %s", best.signature(), method.signature());
                    // could maybe match more as well, should we print that as well?
                    return QMetaMethod();
                }
                best = method;
                continue;
            }
        }
        best = method;
        break;
    }
    return best;
}

Q_DECLARE_METATYPE(QDBusReply<void>);
Q_DECLARE_METATYPE(QDBusReply<int>);
Q_DECLARE_METATYPE(QDBusReply<bool>);
Q_DECLARE_METATYPE(QDBusReply<QString>);
Q_DECLARE_METATYPE(QDBusReply<double>);
Q_DECLARE_METATYPE(QDBusReply<QTime>);
Q_DECLARE_METATYPE(QDBusReply<QDateTime>);
Q_DECLARE_METATYPE(QDBusReply<QDate>);
Q_DECLARE_METATYPE(QDBusReply<QStringList>);
Q_DECLARE_METATYPE(QDBusReply<QRegExp>);
Q_DECLARE_METATYPE(QDBusReply<QList<int> >);

static QHash<int, int> registerMetaTypes()
{
    static QHash<int, int> types;
    if (types.isEmpty()) {
        types[qRegisterMetaType<QDBusReply<void> >("QDBusReply<void>")] = QMetaType::Void;
        types[qRegisterMetaType<QDBusReply<int> >("QDBusReply<int>")] = QMetaType::Int;
        types[qRegisterMetaType<QDBusReply<bool> >("QDBusReply<bool>")] = QMetaType::Bool;
        types[qRegisterMetaType<QDBusReply<QString> >("QDBusReply<QString>")] = QMetaType::QString;
        types[qRegisterMetaType<QDBusReply<double> >("QDBusReply<double>")] = QMetaType::Double;
        types[qRegisterMetaType<QDBusReply<QTime> >("QDBusReply<QTime>")] = QMetaType::QTime;
        types[qRegisterMetaType<QDBusReply<QDateTime> >("QDBusReply<QDateTime>")] = QMetaType::QDateTime;
        types[qRegisterMetaType<QDBusReply<QDate> >("QDBusReply<QDate>")] = QMetaType::QDate;
        types[qRegisterMetaType<QDBusReply<QStringList> >("QDBusReply<QStringList>")] = QMetaType::QStringList;
        types[qRegisterMetaType<QDBusReply<QRegExp> >("QDBusReply<QRegExp>")] = QMetaType::QRegExp;
        types[qRegisterMetaType<QDBusReply<QRegExp> >("QDBusReply<QList<int> >")] = QMetaType::QRegExp;
    }
    return types;
}

static QVariant toVariant(const QVariant &dbusReply)
{
    const QHash<int, int> types = registerMetaTypes();
    const int type = types.value(dbusReply.userType());
    switch (type) {
    case QMetaType::Void: return QVariant();
    case QMetaType::Int: return qVariantValue<QDBusReply<int> >(dbusReply).value();
    case QMetaType::Bool: return qVariantValue<QDBusReply<bool> >(dbusReply).value();
    case QMetaType::QString: return qVariantValue<QDBusReply<QString> >(dbusReply).value();
    case QMetaType::Double: return qVariantValue<QDBusReply<double> >(dbusReply).value();
    case QMetaType::QTime: return qVariantValue<QDBusReply<QTime> >(dbusReply).value();
    case QMetaType::QDateTime: return qVariantValue<QDBusReply<QDateTime> >(dbusReply).value();
    case QMetaType::QDate: return qVariantValue<QDBusReply<QDate> >(dbusReply).value();
    case QMetaType::QStringList: return qVariantValue<QDBusReply<QStringList> >(dbusReply).value();
    case QMetaType::QRegExp: return qVariantValue<QDBusReply<QRegExp> >(dbusReply).value();
    default: qWarning("Unknown type %d %d", dbusReply.userType(), types.value(dbusReply.userType()));
    }
    return QVariant();
}

int main(int argc, char *argv[])
{
    QCoreApplication *coreApp = new QCoreApplication(argc, argv);
    ::initApp(coreApp, "tokoloshhead");
    const QHash<int, int> types = registerMetaTypes();
    TokoloshInterface dbusInterface("com.TokoloshXineBackend.TokoloshMediaPlayer",
                                    "/TokoloshMediaPlayer",
                                    QDBusConnection::sessionBus());

    if (!dbusInterface.QDBusAbstractInterface::isValid() || Config::isEnabled("restartbackend")) {
#ifndef Q_OS_UNIX
#warning Does this stuff work on windows?
#endif
        if (!QProcess::startDetached("tokoloshtail")) {
            QProcess::startDetached("../tokoloshtail/tokoloshtail");
        }
    }

    if (argc > 1) {
        const QMetaObject *dbusInterfaceMetaObject = dbusInterface.metaObject();
        const QStringList args = Config::arguments();
        for (int i=1; i<argc; ++i) {
            const QString &arg = args.at(i);
            if (arg == "--list-methods") {
                for (int j=dbusInterfaceMetaObject->methodOffset(); j<dbusInterfaceMetaObject->methodCount(); ++j) {
                    printf("%s\n", dbusInterfaceMetaObject->method(j).signature());
                }
                return 0;
            }
            const QMetaMethod method = ::findMethod(arg, dbusInterfaceMetaObject);
            if (!method.signature())
                continue;

            QVariant returnArg(static_cast<QVariant::Type>(QMetaType::type(method.typeName())));
            const QList<QByteArray> parameterTypes = method.parameterTypes();
            bool ret = false;
            if (parameterTypes.isEmpty()) {
                ret = method.invoke(&dbusInterface, Qt::DirectConnection,
                                    QGenericReturnArgument(returnArg.typeName(), returnArg.type() ? returnArg.data() : 0));
            } else if (argc - i - 1 < parameterTypes.size()) {
                qWarning("Not enough arguments specified for %s needed %d, got %d",
                         method.signature(), parameterTypes.size(), argc - i - 1);
                return 1; // ### ???
            } else {
                QVariant arguments[10];
                for (int j=0; j<parameterTypes.size(); ++j) {
                    const int type = QMetaType::type(parameterTypes.at(j).constData());
                    arguments[j] = args.at(++i);
                    if (!arguments[j].convert(static_cast<QVariant::Type>(type))) {
                        qWarning("Can't convert %s to %s", qPrintable(args.at(i)), parameterTypes.at(i).constData());
                        return 1; // ### ???
                    }
                }
                ret = method.invoke(&dbusInterface, Qt::DirectConnection,
                                    QGenericReturnArgument(returnArg.typeName(), returnArg.type() ? returnArg.data() : 0),
                                    QGenericArgument(parameterTypes.value(0).constData(), arguments[0].data()),
                                    QGenericArgument(parameterTypes.value(1).constData(), arguments[1].data()),
                                    QGenericArgument(parameterTypes.value(2).constData(), arguments[2].data()),
                                    QGenericArgument(parameterTypes.value(3).constData(), arguments[3].data()),
                                    QGenericArgument(parameterTypes.value(4).constData(), arguments[4].data()),
                                    QGenericArgument(parameterTypes.value(5).constData(), arguments[5].data()),
                                    QGenericArgument(parameterTypes.value(6).constData(), arguments[6].data()),
                                    QGenericArgument(parameterTypes.value(7).constData(), arguments[7].data()),
                                    QGenericArgument(parameterTypes.value(8).constData(), arguments[8].data()),
                                    QGenericArgument(parameterTypes.value(9).constData(), arguments[9].data()));
            }
            if (!ret) {
                qWarning("Can't invoke %s", method.signature());
                return 1;
            } else if (dbusInterface.lastError().type() != QDBusError::NoError) {
                qWarning("Couldn't invoke %s %s %s", method.signature(),
                         qPrintable(dbusInterface.lastError().name()),
                         qPrintable(dbusInterface.lastError().message()));
                return 1;
            } else {
                QFile f;
                f.open(stdout, QIODevice::WriteOnly);
                QDebug out(&f);
                const QVariant var = ::toVariant(returnArg);
                out << "Invoked" << method.signature() << "successfully" << returnArg << var << endl;
                return 0;
            }
        }
    }

    foreach(QString arg, Config::unusedArguments()) {
        const QFileInfo fi(arg);
        if (!fi.exists()) {
            if (!arg.startsWith("-")) {
                qWarning("%s doesn't seem to exist", qPrintable(arg));
            }
            continue;
        }
        dbusInterface.load(arg);
    }
    // ### handle file args
    if (!startGui()) {
        dbusInterface.sendWakeUp();
        return 0;
    }

    delete coreApp;

    QApplication app(argc, argv);
    ::initApp(&app, "tokoloshhead");
    Player player(&dbusInterface);
    const QDBusReply<int> vol = dbusInterface.volume();
    if (dbusInterface.lastError().type() != QDBusError::NoError) {
        if (!QProcess::startDetached("tokoloshtail")) {
            qWarning("Can't start tokoloshtail");

        }
    }
//    qDebug() << dbusInterface.lastError() << dbusInterface.lastError().type();
    if (!player.setSkin(Config::value<QString>("skin", QString(":/skins/dullSod")))) {
        const bool ret = player.setSkin(QLatin1String(":/skins/dullSod"));
        Q_ASSERT(ret);
        Q_UNUSED(ret);
    }
    player.show();
    return app.exec();
}
