######################################################################
# Automatically generated by qmake (2.01a) Fri Apr 10 13:34:37 2009
######################################################################

TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += /Users/anders/tokolosh/tokoloshhead_v2/. .

# Input
HEADERS += player.h tokolosh_interface.h config.h widgets.h
SOURCES += main.cpp player.cpp tokolosh_interface.cpp config.cpp widgets.cpp
CONFIG += qdbus debug
unix {
    MOC_DIR=.moc
    UI_DIR=.ui
    OBJECTS_DIR=.obj
} else {
    MOC_DIR=tmp/moc
    UI_DIR=tmp/ui
    OBJECTS_DIR=tmp/obj
}
linux {
    generateInterface.target = GenerateInterface
    generateInterface.commands = sh generateadaptorfiles.sh
    QMAKE_EXTRA_TARGETS += generateInterface
    PRE_TARGETDEPS+= GenerateInterface
    OBJECTS_DIR = .obj
    MOC_DIR = .moc
    UI_DIR = .ui
}
CONFIG -= app_bundle
QT += dbus
RESOURCES += tokoloshhead_v2.qrc
