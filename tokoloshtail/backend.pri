TEMPLATE = lib
CONFIG += plugin 
DEPENDPATH += .
INCLUDEPATH += .
HEADERS += backend.h backendplugin.h
SOURCES += backend.cpp 
!include(../shared/shared.pri):error("Can't find shared.pri")
DESTDIR = plugins
