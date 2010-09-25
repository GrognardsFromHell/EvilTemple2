
TEMPLATE = lib

TARGET = common

INCLUDEPATH += include/ ../3rdparty/minizip/

HEADERS += include/common/quadtree.h \
    include/common/tga.h \
    include/common/global.h \
    include/common/paths.h \
    include/common/datafileengine.h \
    ../3rdparty/SFMT-src-1.3.3/SFMT.h

SOURCES += src/tga.cpp \
           src/paths.cpp \
           src/datafileengine.cpp \
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
