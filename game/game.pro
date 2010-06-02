
QT += network \
    opengl \
    script \
    scripttools \
    webkit \
    xml \
    declarative

TEMPLATE = lib

TARGET = game

CONFIG += dll

TEMPLE_LIBS += qt3d minizip model glew jpeg
include(../3rdparty/game-math/game-math.pri)

DEFINES += GAME_LIBRARY GAMEMATH_MEMORY_OPERATORS

SOURCES += \
    mainwindow.cpp \
    game.cpp \
    scriptengine.cpp \
    datafileengine.cpp \
    material.cpp \
    renderstates.cpp \
    modelfile.cpp \
    texture.cpp \
    materialstate.cpp \
    glslprogram.cpp \
    gameview.cpp \
    backgroundmap.cpp \
    tga.cpp \
    clippinggeometry.cpp \
    particlesystem.cpp \
    modelinstance.cpp \
    scenenode.cpp \
    scene.cpp \
    entity.cpp \
    renderable.cpp \
    boxrenderable.cpp \
    materialcache.cpp \
    texturesource.cpp \
    lighting.cpp \
    lighting_debug.cpp \
    profiler.cpp \
    profilerdialog.cpp \
    scriptables.cpp
HEADERS += \
    mainwindow.h \
    game.h \
    stable.h \
    util.h \
    scriptengine.h \
    datafileengine.h \
    gameglobal.h \
    material.h \
    renderstates.h \
    modelfile.h \
    texturesource.h \
    texture.h \
    materialstate.h \
    glslprogram.h \
    gameview.h \
    backgroundmap.h \
    tga.h \
    clippinggeometry.h \
    particlesystem.h \
    modelinstance.h \
    scenenode.h \
    scene.h \
    renderqueue.h \
    entity.h \
    renderable.h \
    boxrenderable.h \
    materialcache.h \
    lighting.h \
    lighting_debug.h \
    drawhelper.h \
    profiler.h \
    profilerdialog.h \
    scriptables.h
OTHER_FILES += map_material.xml \
    clipping_material.xml \
    lighting.vs \
    lighting.fs

INCLUDEPATH += .

PRECOMPILED_HEADER = stable.h

win32:LIBS += -lpsapi

include(../base.pri)

RESOURCES += \
    resources.qrc

FORMS += \
    profilerdialog.ui
