#include <QApplication>
#include <QFileInfo>
#include "player.h"
#include "config.h"
#include "tokolosh_interface.h"

int main(int argc, char * argv[])
{
    QApplication app(argc,argv);
    app.setApplicationName("tokoloshhead_v2");

    TokoloshInterface dbusInterface("com.TokoloshXineBackend.TokoloshMediaPlayer",
                                    "/TokoloshMediaPlayer",
                                    QDBusConnection::sessionBus(),
                                    &app);

    int queuedMethodIndex = -1;
    const QMetaObject *dbusInterfaceMetaObject = dbusInterface.metaObject();

    foreach(QString arg, Config::unusedArguments()) {
        if ((queuedMethodIndex == -1) &&
            QFile::exists(arg))
        {
            // does QFile match for dirs?
            //If dir: add path + scan
            //If file, load file?
            QString consideredPath(arg);
            while(QFileInfo(consideredPath).isSymLink()) {
                //FIXME: might want to have a counter incase there is a cyclical symlink
                consideredPath = QFileInfo(consideredPath).symLinkTarget();
            }
            if(QFileInfo(consideredPath).isFile()){
                dbusInterface.load(arg);
            } else {
                //handle me
                //path loading not in interface! investigating seperately
            }
        } else if (arg.startsWith('-')) {
            queuedMethodIndex = -1;
            const char* charArg = arg.endsWith("()")
                            ? arg.toAscii().constData()
                            : arg.append("()").toAscii().constData();
            //char*+1 is a nasty but efficient way to drop the first char
            const int index = dbusInterfaceMetaObject->indexOfSlot(charArg+1);
            if(index != -1) {
                qWarning("%s has index %d",charArg+1,index);
                QMetaMethod calledMethod = dbusInterfaceMetaObject->method(index);
                if(calledMethod.parameterNames().size() > 0) {
                    //gag for your arg beatch
                    queuedMethodIndex = index;
                } else {
                    calledMethod.invoke( &dbusInterface,
                            Qt::DirectConnection,
                            QGenericReturnArgument());
                    
                    app.quit();
                    return 0;
                }
            } else {
                qWarning("Unknown argument %s", qPrintable(arg));
            }
        } else {
            if(queuedMethodIndex!=-1)
            {
                QMetaMethod calledMethod = dbusInterfaceMetaObject->method(queuedMethodIndex);
                calledMethod.invoke( &dbusInterface,
                        Qt::DirectConnection,
                        QGenericReturnArgument(),
                        Q_ARG(QString, arg));

                app.quit();
                return 0;
            }
            qWarning("%s doesn't seem to exist", qPrintable(arg));
            queuedMethodIndex = -1;
        }
    }
    Player player(&dbusInterface);
    if (!player.setSkin(Config::value<QString>("skin", QString(":/skins/dullSod")))) {
        const bool ret = player.setSkin(QLatin1String(":/skins/dullSod"));
        Q_ASSERT(ret);
        Q_UNUSED(ret);
    }
    player.show();
    return app.exec();
}
/*
   Donald Carr (sirspudd_at_gmail.com) plays songs occasionally
   Copyright (C) 2007 Donald Car

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNES FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MAction 02110-1301 USAction.
   */
