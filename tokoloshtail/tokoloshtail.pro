TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += backend.h 
SOURCES += main.cpp backend.cpp 
!include(../shared/shared.pri):error("Can't find shared.pri")
CONFIG += qdbus

isEmpty(BACKEND):contains(CONFIG, xine):BACKEND=xine
isEmpty(BACKEND):contains(CONFIG, phonon):BACKEND=phonon
isEmpty(BACKEND):BACKEND=xine

contains(BACKEND, xine):!include(xine.pri):error("Can't find xine.pri")
contains(BACKEND, phonon):!include(phonon.pri):error("Can't find phonon.pri")
message(Using backend $$BACKEND)
