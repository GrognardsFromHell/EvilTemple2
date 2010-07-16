
TEMPLATE = lib
TARGET = binkplayer
DEFINES += BINKPLAYER_LIBRARY

PRECOMPILED_HEADER = stable.h

include(../3rdparty/game-math/game-math.pri)

TEMPLE_LIBS += audioengine openal libavcodec

HEADERS += \
    stable.h \
    binkplayerglobal.h \
    binkplayer.h

SOURCES += \
    binkplayer.cpp

include(../base.pri)
