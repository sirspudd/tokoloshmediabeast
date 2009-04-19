TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += player.h \
           config.h \
           widgets.h \
           tokolosh_interface.h \
           log.h

SOURCES += main.cpp \
           player.cpp \
           config.cpp \
           widgets.cpp \
           tokolosh_interface.cpp \
           log.cpp

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

unix {
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
