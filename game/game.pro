
QT += network \
    opengl \
    script \
    scripttools \
    webkit \
    xml \
    declarative

TEMPLATE = lib

TARGET = game

TEMPLE_LIBS += qt3d minizip model game-math

DEFINES += GAME_LIBRARY

SOURCES += \
    ui/mainwindow.cpp \
    ui/gamegraphicsscene.cpp \
    game.cpp \
    ui/gamegraphicsview.cpp \
    camera.cpp \
    ui/consolewidget.cpp \
    scriptengine.cpp \
    datafileengine.cpp \
    material.cpp \
    renderstates.cpp
HEADERS += \
    ui/mainwindow.h \
    ui/gamegraphicsscene.h \
    game.h \
    stable.h \
    io/basepathfinder.h \
    ui/gamegraphicsview.h \
    camera.h \
    util.h \
    ui/consolewidget.h \
    scriptengine.h \
    datafileengine.h \
    gameglobal.h \
    material.h \
    renderstates.h
OTHER_FILES +=

INCLUDEPATH += .

PRECOMPILED_HEADER = stable.h

# Win32 specific code
win32:SOURCES += io/basepathfinder_win32.cpp
else:SOURCES += io/basepathfinder.cpp
win32:LIBS += -ladvapi32 -lpsapi

include(../base.pri)
