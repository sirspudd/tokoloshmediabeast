HEADERS += xinebackend.h
SOURCES += xinebackend.cpp
DEFINES += XINE_STREAM_COUNT=0 BACKEND=XineBackend
LIBS += -lxine
macx {
    INCLUDEPATH+=/opt/local/include
    LIBS += -L/opt/local/lib
}
linux {
    LIBS += -L/usr/lib lz -lnsl -lpthread -lrt
    QMAKE_CXXFLAGS += -fPIC
    QMAKE_CFLAGS += -fPIC
}
include(backend.pri)
