TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += player.h \
           config.h \
           widgets.h \
           tokolosh_interface.h

SOURCES += main.cpp \
           player.cpp \
           config.cpp \
           widgets.cpp \
           tokolosh_interface.cpp

CONFIG += qdbus debug

unix {
    generateInterface.target = GenerateInterface
    generateInterface.commands = sh generateadaptorfiles.sh
    QMAKE_EXTRA_TARGETS += generateInterface
    PRE_TARGETDEPS+= GenerateInterface
    OBJECTS_DIR = .obj
    MOC_DIR = .moc
    UI_DIR = .ui
}

QT += dbus
RESOURCES += tokoloshhead_v2.qrc
!include(../shared/shared.pri):error("Can't find shared.pri")
