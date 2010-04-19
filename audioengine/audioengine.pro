
TEMPLATE = lib
TARGET = audioengine

DEFINES += AUDIOENGINE_LIBRARY

TEMPLE_LIBS += openal

win32:INCLUDEPATH += ../3rdparty/libavcodec/include
LIBS += -L../3rdparty/libavcodec/lib -lavcodec -lavformat -lavutil

PRECOMPILED_HEADER = stable.h

HEADERS += \
    audioengineglobal.h \
    stable.h \
    audioengine.h \
    isoundsource.h \
    soundformat.h \
    isound.h \
    isoundhandle.h \
    mp3reader.h \
    wavereader.h \
    scripting.h

SOURCES += \
    audioengine.cpp \
    isound.cpp \
    isoundsource.cpp \
    isoundhandle.cpp \
    mp3reader.cpp \
    wavereader.cpp \
    soundformat.cpp \
    scripting.cpp

include(../base.pri)
