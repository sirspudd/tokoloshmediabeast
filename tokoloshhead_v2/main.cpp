#include <QApplication>
#include <QFileInfo>
#include "player.h"
#include "config.h"
#include "tokolosh_interface.h"

static inline QFileInfo resolveSymlink(const QString &file)
{
    QFileInfo fi(file);
    QStringList paths;
    forever {
        if (paths.contains(fi.absoluteFilePath())) {
            qWarning("%s is a recursive symlink", qPrintable(paths.join(" => ")));
            return QFileInfo();
        }
        paths.append(fi.absoluteFilePath());
        if (!fi.isSymLink()) {
            break;
        }
        fi = fi.symLinkTarget();
    }
    return fi;
}

static inline QStringList toFiles(const QString &arg, bool recursive = false)
{
    const QFileInfo fi = ::resolveSymlink(arg);
    QStringList files;
    if (fi.isDir()) {
        const QDir dir(fi.absoluteFilePath());
        foreach(QString file, dir.entryList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot)) {
            const QFileInfo f = ::resolveSymlink(file);
            if (!f.isDir()) {
                files.append(f.absoluteFilePath());
            } else if (recursive) {
                files += ::toFiles(f.absoluteFilePath(), recursive);
            }
        }
    } else if (fi.isFile()) {
        files.append(fi.absoluteFilePath());
    }
    return files;
}

static inline QMetaMethod findMethod(QString arg, const QMetaObject *metaObject)
{
    QRegExp rx("^-*");
    arg.remove(rx);
    const int methodCount = metaObject->methodCount();
    for (int i=metaObject->methodOffset(); i<methodCount; ++i) {
        const QMetaMethod method = metaObject->method(i);
        if (method.methodType() != QMetaMethod::Slot
            && method.methodType() != QMetaMethod::Method)
            continue;

        const QString methodName = method.signature();
        qDebug() << method.typeName() << methodName;
        if (!methodName.startsWith(arg))
            continue;

        if (methodName.size() != arg.size()) {
            // if the argument part is not specified that's still a match
            const int index = methodName.lastIndexOf("(");
            if (arg.size() < index) // if you specify setVo that shouldn't match setVolume(or should it?)
                continue;
        }
        return method;
    }
    return QMetaMethod();
}

static void registerMetaTypes()
{
    qRegisterMetaType<QDBusReply<void> >("QDBusReply<void>");
    qRegisterMetaType<QDBusReply<int> >("QDBusReply<int>");
    qRegisterMetaType<QDBusReply<bool> >("QDBusReply<bool>");
    qRegisterMetaType<QDBusReply<QString> >("QDBusReply<QString>");
    qRegisterMetaType<QDBusReply<double> >("QDBusReply<double>");
    qRegisterMetaType<QDBusReply<QTime> >("QDBusReply<QTime>");
    qRegisterMetaType<QDBusReply<QDateTime> >("QDBusReply<QDateTime>");
    qRegisterMetaType<QDBusReply<QDate> >("QDBusReply<QDate>");
    qRegisterMetaType<QDBusReply<QStringList> >("QDBusReply<QStringList>");
    qRegisterMetaType<QDBusReply<QRegExp> >("QDBusReply<QRegExp>");
    qRegisterMetaType<QDBusReply<QVariant> >("QDBusReply<QVariant>");
}


int main(int argc, char *argv[])
{
    registerMetaTypes();
    TokoloshInterface dbusInterface("com.TokoloshXineBackend.TokoloshMediaPlayer",
                                    "/TokoloshMediaPlayer",
                                    QDBusConnection::sessionBus());
    if (argc > 1) {
        QCoreApplication *coreApp = new QCoreApplication(argc, argv);
        coreApp->setApplicationName("tokoloshhead_v2");

        const QMetaObject *dbusInterfaceMetaObject = dbusInterface.metaObject();
        const QStringList args = coreApp->arguments();
        for (int i=1; i<argc; ++i) {
            const QString &arg = args.at(i);
            const QMetaMethod method = ::findMethod(arg, dbusInterfaceMetaObject);
            if (!method.signature())
                continue;

            QVariant returnArg(static_cast<QVariant::Type>(QMetaType::type(method.typeName())));
            const QList<QByteArray> types = method.parameterTypes();
            bool ret = false;
            if (types.isEmpty()) {
                ret = method.invoke(&dbusInterface, Qt::DirectConnection,
                                    QGenericReturnArgument(returnArg.typeName(), returnArg.data()));
            } else if (argc - i - 1 < types.size()) {
                qWarning("Not enough arguments specified for %s needed %d, got %d",
                         method.signature(), types.size(), argc - i - 1);
                return false; // ### ???
            } else {
                QVariant arguments[10];
                for (int j=0; j<types.size(); ++j) {
                    const int type = QMetaType::type(types.at(j).constData());
                    arguments[j] = args.at(++i);
                    if (!arguments[j].convert(static_cast<QVariant::Type>(type))) {
                        qWarning("Can't convert %s to %s", qPrintable(args.at(i)), types.at(i).constData());
                        return false; // ### ???
                    }
                }
                QDBusReply<void> rep;
                ret = method.invoke(&dbusInterface, Qt::DirectConnection,
                                    QGenericReturnArgument(returnArg.typeName(), returnArg.data()),
                                    QGenericArgument(types.value(0).constData(), arguments[0].data()),
                                    QGenericArgument(types.value(1).constData(), arguments[1].data()),
                                    QGenericArgument(types.value(2).constData(), arguments[2].data()),
                                    QGenericArgument(types.value(3).constData(), arguments[3].data()),
                                    QGenericArgument(types.value(4).constData(), arguments[4].data()),
                                    QGenericArgument(types.value(5).constData(), arguments[5].data()),
                                    QGenericArgument(types.value(6).constData(), arguments[6].data()),
                                    QGenericArgument(types.value(7).constData(), arguments[7].data()),
                                    QGenericArgument(types.value(8).constData(), arguments[8].data()),
                                    QGenericArgument(types.value(9).constData(), arguments[9].data()));
            }
            if (!ret) {
                qWarning("Can't invoke %s", method.signature());
            } else {
                QFile f;
                f.open(stdout, QIODevice::WriteOnly);
                QDebug out(&f);
                out << "Invoked" << method.signature() << "successfully" << returnArg << endl;
                return 0;
            }
        }
delete coreApp;
    }
//             }

//             if (QFileInfo(consideredPath).isFile()) {
//                 dbusInterface.load(arg);
//             } else {
//                 //handle me
//                 //path loading not in interface! investigating seperately
//             }
//         } else if (arg.startsWith('-')) {
//             queuedMethodIndex = -1;
//             QByteArray sig = arg;
//             const char* charArg = arg.endsWith("()")
//                                   ? arg.toAscii().constData()
//                                   : arg.append("()").toAscii().constData();
//             //char*+1 is a nasty but efficient way to drop the first char
//             const int index = dbusInterfaceMetaObject->indexOfSlot(charArg+1);
//             if (index != -1) {
//                 qWarning("%s has index %d",charArg+1,index);
//                 QMetaMethod calledMethod = dbusInterfaceMetaObject->method(index);
//                 if (calledMethod.parameterNames().size() > 0) {
//                     //gag for your arg beatch
//                     queuedMethodIndex = index;
//                 } else {
//                     calledMethod.invoke(&dbusInterface,
//                                         Qt::DirectConnection,
//                                         QGenericReturnArgument());

//                     app.quit();
//                     return 0;
//                 }
//             } else {
//                 qWarning("Unknown argument %s", qPrintable(arg));
//             }
//         } else {
//             if (queuedMethodIndex!=-1) {
//                 QMetaMethod calledMethod = dbusInterfaceMetaObject->method(queuedMethodIndex);
//                 calledMethod.invoke(&dbusInterface,
//                                     Qt::DirectConnection,
//                                     QGenericReturnArgument(),
//                                     Q_ARG(QString, arg));

//                 app.quit();
//                 return 0;
//             }
//             qWarning("%s doesn't seem to exist", qPrintable(arg));
//             queuedMethodIndex = -1;
//         }
//     }
    QApplication app(argc, argv);
    app.setApplicationName("tokoloshhead_v2");
    Player player(&dbusInterface);
    if (!player.setSkin(Config::value<QString>("skin", QString(":/skins/dullSod")))) {
        const bool ret = player.setSkin(QLatin1String(":/skins/dullSod"));
        Q_ASSERT(ret);
        Q_UNUSED(ret);
    }
    player.show();
    return app.exec();
}
