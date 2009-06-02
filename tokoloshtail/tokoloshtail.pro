TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += backend.h xinebackend.h  ../shared/tokolosh_adaptor.h
SOURCES += main.cpp xinebackend.cpp backend.cpp ../shared/tokolosh_adaptor.cpp
!include(../shared/shared.pri):error("Can't find shared.pri")
CONFIG += qdbus

DEFINES += XINE_STREAM_COUNT=0
