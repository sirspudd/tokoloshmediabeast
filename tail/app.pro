TEMPLATE = app
TARGET = tokoloshtail
DESTDIR = ../bin
DEFINES += PLUGINDIR="\"$$PWD/plugins\""
warning("FixMe: I can't seem to figure out how to pass in a quoted define")
DEPENDPATH += .
INCLUDEPATH += .
SOURCES += main.cpp tail.cpp
HEADERS += tail.h backend.h taginterface.h id3taginterface.h

!include(../shared/shared.pri):error("Can't find shared.pri")
CONFIG += qdbus
LIBS += -lid3
OBJECTS_DIR = .objapp
