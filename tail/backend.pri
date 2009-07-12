TEMPLATE = lib
CONFIG += plugin
DEPENDPATH += .
INCLUDEPATH += .
HEADERS += backend.h backendplugin.h
!include(../shared/shared.pri):error("Can't find shared.pri")
DESTDIR = ../plugins
DEFINES += THREADED_RECURSIVE_LOAD
