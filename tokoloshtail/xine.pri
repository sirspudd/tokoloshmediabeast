HEADERS += $$PWD/xinebackend.h 
SOURCES += $$PWD/xinebackend.cpp
DEFINES += XINE_STREAM_COUNT=0 BACKEND=XineBackend XINEBACKEND
LIBS += -lxine 
macx {
    INCLUDEPATH+=/opt/local/include
    LIBS += -L/opt/local/lib
}
linux {
    LIBS += -L/usr/lib lz -lnsl -lpthread -lrt
}

 #LIBS += -L/usr/lib -lxine -lz -lnsl -lpthread -lrt
