TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += backend.h xinebackend.h 
SOURCES += main.cpp xinebackend.cpp backend.cpp 
!include(../shared/shared.pri):error("Can't find shared.pri")
CONFIG += qdbus

DEFINES += XINE_STREAM_COUNT=0
