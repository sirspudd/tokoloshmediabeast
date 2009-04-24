INCLUDEPATH += $$PWD
HEADERS += $$PWD/log.h $$PWD/global.h
SOURCES += $$PWD/log.cpp
unix {
    MOC_DIR=.moc
    UI_DIR=.ui
    OBJECTS_DIR=.obj
} else {
    MOC_DIR=tmp/moc
    UI_DIR=tmp/ui
    OBJECTS_DIR=tmp/obj
}

CONFIG -= app_bundle
