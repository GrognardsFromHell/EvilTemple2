TARGET = qtga
include(qpluginbase.pri)
HEADERS += qtgahandler.h \
    qtgafile.h
SOURCES += main.cpp \
    qtgahandler.cpp \
    qtgafile.cpp
DESTDIR = ../../bin/imageformats
