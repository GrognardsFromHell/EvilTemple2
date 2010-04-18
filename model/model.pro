
TEMPLATE = lib
TARGET = model

include(../base.pri)

DEFINES += MODEL_LIBRARY

HEADERS += \
    campaign.h \
    savegame.h \
    savegames.h \
    modelglobal.h

SOURCES += \
    campaign.cpp \
    savegame.cpp \
    savegames.cpp
