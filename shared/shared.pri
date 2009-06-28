INCLUDEPATH += $$PWD
unix {
    MOC_DIR=.moc
    UI_DIR=.ui
    OBJECTS_DIR=.obj
} else {
    MOC_DIR=tmp/moc
    UI_DIR=tmp/ui
    OBJECTS_DIR=tmp/obj
}

mac:CONFIG -= app_bundle
unix {
    LIBS += -L/usr/lib -lxine -lz -lnsl -lpthread -lrt
    #generateadaptor.target = $$PWD/tokolosh_adaptor.h 
    #generateadaptor.commands = sh $$PWD/generatedbusinterfaces.sh $$PWD
    #generateinterface.target = $$PWD/tokolosh_interface.h 
    #generateinterface.commands = sh $$PWD/generatedbusinterfaces.sh $$PWD
    #QMAKE_EXTRA_TARGETS += generateadaptor generateinterface
    #PRE_TARGETDEPS += $$PWD/tokolosh.xml $$PWD/tokolosh_adaptor.h $$PWD/tokolosh_interface.h
}
HEADERS += $$PWD/log.h $$PWD/global.h $$PWD/config.h 
SOURCES += $$PWD/log.cpp $$PWD/config.cpp $$PWD/global.cpp
