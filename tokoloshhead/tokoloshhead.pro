TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += player.h \
           config.h \
           widgets.h \
           shortcutdialog.h \
           SkinSelectionDialog.h \
           tokolosh_interface.h \
           resizer.h

SOURCES += main.cpp \
           player.cpp \
           config.cpp \
           widgets.cpp \
           shortcutdialog.cpp \
           SkinSelectionDialog.cpp \
           tokolosh_interface.cpp

CONFIG += qdbus debug

unix {
    generateInterface.target = tokolosh_interface.cpp
    generateInterface.commands = sh generateadaptorfiles.sh
    QMAKE_EXTRA_TARGETS += generateInterface
    PRE_TARGETDEPS += ../tokoloshtail/tokolosh.xml
    OBJECTS_DIR = .obj
    MOC_DIR = .moc
    UI_DIR = .ui
}

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
