
TEMPLATE = lib
TARGET = freealut

include(../../base.pri)

DESTDIR = ../../bin

INCLUDEPATH += $$PWD/include
win32:INCLUDEPATH += $$PWD/../openal-1.1/include
win32:LIBS += -L$$PWD/../openal-1.1/libs/Win32/ -lOpenAL32

# We're building the dll
DEFINES += ALUT_BUILD_LIBRARY

win32:INCLUDEPATH += $$PWD/win32/
win32:DEFINES += HAVE_CONFIG_H
win32:HEADERS += win32/config.h

win32-msvc2008:DEFINES += _CRT_SECURE_NO_WARNINGS

HEADERS += include/AL/alut.h \
    src/alutInternal.h

SOURCES += src/alutBufferData.c \
    src/alutCodec.c \
    src/alutError.c \
    src/alutInit.c \
    src/alutInputStream.c \
    src/alutLoader.c \
    src/alutOutputStream.c \
    src/alutUtil.c \
    src/alutVersion.c \
    src/alutWaveform.c
