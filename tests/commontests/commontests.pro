#-------------------------------------------------
#
# Project created by QtCreator 2010-09-15T21:58:28
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_commontest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_commontest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

include(../../common/common.pri)
