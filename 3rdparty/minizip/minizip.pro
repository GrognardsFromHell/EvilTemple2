
TEMPLATE = lib
TARGET = minizip
DESTDIR = ../../bin

CONFIG(debug, debug|release) {
 TARGET = $$join(TARGET,,,_d)
 CONFIG += warn_on
} else {
 CONFIG += warn_off
}

DEFINES += MINIZIP_LIBRARY

SOURCES += ioapi.c \
    unzip.c \
    zip.c \
    zipwriter.cpp

win32:SOURCES += iowin32.c

HEADERS += crypt.h \
    ioapi.h \
    unzip.h \
    zip.h \
    zlib.h \
    zconf.h \
    zipwriter.h \
    minizipglobal.h

win32:HEADERS += iowin32.h
