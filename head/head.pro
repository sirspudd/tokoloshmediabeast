TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR = ../bin
TARGET = tokoloshhead

HEADERS += player.h \
           widgets.h \
           shortcutdialog.h \
           skinselectiondialog.h \
           model.h \
           playlist.h \
           resizer.h \
           ../shared/global.h \
           dbusinterface.h

SOURCES += main.cpp \
           player.cpp \
           widgets.cpp \
           shortcutdialog.cpp \
           model.cpp \
           skinselectiondialog.cpp \
           dbusinterface.cpp \
           playlist.cpp

CONFIG += qdbus debug


i18n.target = translations
i18n.commands = lupdate head.pro
QMAKE_EXTRA_TARGETS += i18n

i18nrelease.target = translations-release
i18nrelease.commands = lrelease head.pro
QMAKE_EXTRA_TARGETS += i18nrelease

TRANSLATIONS += i18n/head.no.ts \
		i18n/head.de.ts

QT += dbus
RESOURCES += head.qrc
include(../shared/shared.pri)
