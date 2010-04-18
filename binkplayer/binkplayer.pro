
TEMPLATE = lib
TARGET = binkplayer
DEFINES += BINKPLAYER_LIBRARY

PRECOMPILED_HEADER = stable.h

TEMPLE_LIBS += audioengine openal

win32:INCLUDEPATH += ../3rdparty/libavcodec/include
LIBS += -L../3rdparty/libavcodec/lib -lavcodec -lavformat -lavutil -lswscale

HEADERS += \
    stable.h \
    binkplayerglobal.h \
    binkplayer.h

SOURCES += \
    binkplayer.cpp \
    avcodecregistration.cpp

include(../base.pri)
