#include <QApplication>
#include "player.h"
#include "config.h"

int main(int argc, char * argv[])
{
    QApplication app(argc,argv);
    app.setApplicationName("tokoloshhead_v2");
    Player player;
    if (!player.setSkin(Config::value<QString>("skin", QString(":/skins/dullSod")))) {
        const bool ret = player.setSkin(QLatin1String(":/skins/dullSod"));
        Q_ASSERT(ret);
        Q_UNUSED(ret);
    }
    foreach(QString arg, Config::unusedArguments()) {
        if (QFile::exists(arg)) { // does QFile match for dirs?
            qDebug() << "need to add" << arg;
        } else if (arg.startsWith('-')) {
            qWarning("Unknown argument %s", qPrintable(arg));
        } else {
            qWarning("%s doesn't seem to exist", qPrintable(arg));
        }
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
