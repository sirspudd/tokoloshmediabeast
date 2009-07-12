TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = xinebackend.pro phononbackend.pro app.pro
unix:system(mkdir -p $$PWD/../plugins)
win:system(md $$PWD/../plugins)
