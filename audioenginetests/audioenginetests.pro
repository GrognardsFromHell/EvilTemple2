
TEMPLATE = app

TEMPLE_LIBS += audioengine

CONFIG += console

SOURCES += audioenginetests.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

include(../base.pri)

FORMS += \
    mainwindow.ui
