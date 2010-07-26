#-------------------------------------------------
#
# Project created by QtCreator 2010-07-26T19:48:43
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = dagviewer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    testglwidget.cpp \
    editdialog.cpp

HEADERS  += mainwindow.h \
    testglwidget.h \
    util.h \
    editdialog.h

FORMS    += mainwindow.ui \
    editdialog.ui

include(../../3rdparty/game-math/game-math.pri)
