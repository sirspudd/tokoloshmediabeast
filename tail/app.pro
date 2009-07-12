TEMPLATE = app
TARGET = tokoloshtail
DESTDIR = ../bin
DEFINES += PLUGINDIR="\"$$PWD/plugins\""
warning("FixMe: I can't seem to figure out how to pass in a quoted define")
DEPENDPATH += .
INCLUDEPATH += .
SOURCES += main.cpp
!include(../shared/shared.pri):error("Can't find shared.pri")
CONFIG += qdbus

