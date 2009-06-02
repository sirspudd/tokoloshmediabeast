TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += tokolosh_adaptor.h backend.h xinebackend.h
SOURCES += main.cpp tokolosh_adaptor.cpp xinebackend.cpp backend.cpp
!include(../shared/shared.pri):error("Can't find shared.pri")
CONFIG += qdbus

unix {
    LIBS += -L/usr/lib -lxine -lz -lnsl -lpthread -lrt
    generateadaptor.target = tokolosh_adaptor.cpp
    generateadaptor.commands = sh generateadaptorfiles.sh
    QMAKE_EXTRA_TARGETS += generateadaptor
    PRE_TARGETDEPS += tokolosh.xml tokolosh_adaptor.cpp
    OBJECTS_DIR = .obj
    MOC_DIR = .moc
    UI_DIR = .ui
}
DEFINES += XINE_STREAM_COUNT=0
