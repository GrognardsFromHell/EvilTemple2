
QT += network \
    opengl \
    script \
    scripttools \
    webkit \
    xml \
    declarative

TEMPLATE = lib

TARGET = game

TEMPLE_LIBS += qt3d minizip model glew jpeg
include(../3rdparty/game-math/game-math.pri)

DEFINES += GAME_LIBRARY

SOURCES += \
    ui/mainwindow.cpp \
    ui/gamegraphicsscene.cpp \
    game.cpp \
    ui/gamegraphicsview.cpp \
    camera.cpp \
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
    profilerdialog.cpp
HEADERS += \
    ui/mainwindow.h \
    ui/gamegraphicsscene.h \
    game.h \
    stable.h \
    io/basepathfinder.h \
    ui/gamegraphicsview.h \
    camera.h \
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
    profilerdialog.h
OTHER_FILES += materialfile.xsd \
    map_material.xml \
    clipping_material.xml

INCLUDEPATH += .

PRECOMPILED_HEADER = stable.h

# Win32 specific code
win32:SOURCES += io/basepathfinder_win32.cpp
else:SOURCES += io/basepathfinder.cpp
win32:LIBS += -ladvapi32 -lpsapi

include(../base.pri)

RESOURCES += \
    resources.qrc

FORMS += \
    profilerdialog.ui
