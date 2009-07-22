infile(config.pri, SOLUTIONS_LIBRARY, yes): CONFIG += qtanimationframework-uselib
TEMPLATE += fakelib
QTANIMATIONFRAMEWORK_LIBNAME = $$qtLibraryTarget(QtSolutions_AnimationFramework-2.3)
TEMPLATE -= fakelib
QTANIMATIONFRAMEWORK_LIBDIR = $$PWD/../../lib
unix:qtanimationframework-uselib:!qtanimationframework-buildlib:QMAKE_RPATHDIR += $$QTANIMATIONFRAMEWORK_LIBDIR
