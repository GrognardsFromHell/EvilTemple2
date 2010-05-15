
TEMPLATE = lib
TARGET = binkplayer
DEFINES += BINKPLAYER_LIBRARY

PRECOMPILED_HEADER = stable.h

TEMPLE_LIBS += audioengine openal libavcodec

HEADERS += \
    stable.h \
    binkplayerglobal.h \
    binkplayer.h

SOURCES += \
    binkplayer.cpp \
    avcodecregistration.cpp

include(../base.pri)
