TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += player.h widgets.h shortcutdialog.h skinselectiondialog.h model.h playlist.h resizer.h ../shared/global.h

SOURCES += main.cpp \
           player.cpp \
           widgets.cpp \
           shortcutdialog.cpp \
           model.cpp \
           skinselectiondialog.cpp

CONFIG += qdbus debug


i18n.target = translations
i18n.commands = lupdate tokoloshhead.pro
QMAKE_EXTRA_TARGETS += i18n

i18nrelease.target = translations-release
i18nrelease.commands = lrelease tokoloshhead.pro
QMAKE_EXTRA_TARGETS += i18nrelease

TRANSLATIONS += i18n/tokoloshhead.no.ts \
		i18n/tokoloshhead.de.ts

QT += dbus
RESOURCES += tokoloshhead.qrc
!include(../shared/shared.pri):error("Can't find shared.pri")
