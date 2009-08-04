TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = app.pro
!no_phonon:SUBDIRS += phononbackend.pro 
!no_xine:SUBDIRS += xinebackend.pro 
unix:system(mkdir -p $$PWD/../plugins)
win:system(md $$PWD/../plugins)
linux {
    QMAKE_CFLAGS += -fPIC
    QMAKE_CXXFLAGS += -fPIC
}
