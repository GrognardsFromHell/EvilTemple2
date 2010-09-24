
TEMPLATE = lib

TARGET = common

HEADERS += common/quadtree.h \
    common/tga.h \
    common/global.h \
    ../3rdparty/SFMT-src-1.3.3/SFMT.h

SOURCES += tga.cpp \
           ../3rdparty/SFMT-src-1.3.3/SFMT.c

DEFINES += MINIZIP_LIBRARY

SOURCES += ../3rdparty/minizip/ioapi.c \
    ../3rdparty/minizip/unzip.c \
    ../3rdparty/minizip/zip.c \
    ../3rdparty/minizip/zipwriter.cpp

win32:SOURCES += ../3rdparty/minizip/iowin32.c

HEADERS += ../3rdparty/minizip/crypt.h \
    ../3rdparty/minizip/ioapi.h \
    ../3rdparty/minizip/unzip.h \
    ../3rdparty/minizip/zip.h \
    ../3rdparty/minizip/zlib.h \
    ../3rdparty/minizip/zconf.h \
    ../3rdparty/minizip/zipwriter.h \
    ../3rdparty/minizip/minizipglobal.h

win32:HEADERS += ../3rdparty/minizip/iowin32.h

TEMPLE_LIBS += glew

OTHER_FILES += common.pri

DEFINES += COMMON_LIBRARY

include(../base.pri)
