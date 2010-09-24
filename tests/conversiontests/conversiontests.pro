#-------------------------------------------------
#
# Project created by QtCreator 2010-09-18T00:12:10
#
#-------------------------------------------------

QT       += testlib

TARGET = tst_sectorconversiontest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_sectorconversiontest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

include(../../conversion/conversion.pri)
include(../../troikaformats/troikaformats.pri)
include(../../base.pri)
