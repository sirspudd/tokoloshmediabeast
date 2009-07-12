TEMPLATE = lib
SOURCES = phononbackend.cpp 
HEADERS = phononbackend.h 
QT += phonon
DEFINES += BACKEND=PhononBackend PHONONBACKEND
!include(backend.pri):error("Can't find backend.pri")
