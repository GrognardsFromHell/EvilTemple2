
TEMPLATE = app

TEMPLE_LIBS += audioengine

CONFIG += console

include(../3rdparty/game-math/game-math.pri)

QT += script

SOURCES += audioenginetests.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

include(../base.pri)

FORMS += \
    mainwindow.ui
