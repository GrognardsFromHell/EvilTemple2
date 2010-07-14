
TEMPLATE = lib
TARGET = audioengine

DEFINES += AUDIOENGINE_LIBRARY

TEMPLE_LIBS += openal libavcodec

include(../3rdparty/game-math/game-math.pri)

QT += script

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
    scripting.h \
    scripting_p.h

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
