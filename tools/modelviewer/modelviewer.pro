
QT       += core gui opengl

TARGET = modelviewer
TEMPLATE = app

SOURCES += modelviewer.cpp\
        mainwindow.cpp \
    viewer.cpp \
    editdialog.cpp \
    ../../game/modelfilereader.cpp \
    ../../game/modelfilechunks.cpp \
    ../../game/skeleton.cpp \
    bonehierarchy.cpp \
    ../../game/bindingpose.cpp \
    ../../game/animation.cpp \
    animationsdialog.cpp

HEADERS  += mainwindow.h \
    viewer.h \
    util.h \
    editdialog.h \
    ../../game/modelfilereader.h \
    ../../game/modelfilechunks.h \
    ../../game/skeleton.h \
    bonehierarchy.h \
    ../../game/bindingpose.h \
    ../../game/animation.h \
    animationsdialog.h \
    animinfo.h

FORMS    += mainwindow.ui \
        editdialog.ui \
    bonehierarchy.ui \
    animationsdialog.ui

include(../../3rdparty/game-math/game-math.pri)
