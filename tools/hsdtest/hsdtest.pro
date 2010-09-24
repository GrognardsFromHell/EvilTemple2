#-------------------------------------------------
#
# Project created by QtCreator 2010-07-23T02:40:42
#
#-------------------------------------------------

QT       += core

TARGET = hsdtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

TEMPLE_LIBS += troikaformats

SOURCES += \
    hsdtest.cpp

include(../base.pri)
include(../3rdparty/game-math/game-math.pri)
