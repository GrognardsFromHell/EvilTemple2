
QT += network \
    opengl \
    script \
    scripttools \
    xml \
    declarative

TEMPLATE = lib

TARGET = game

CONFIG += dll

TEMPLE_LIBS += qt3d minizip glew jpeg audioengine qjson binkplayer
include(../3rdparty/game-math/game-math.pri)

DEFINES += GAME_LIBRARY

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
    texturesource.cpp \
    lighting.cpp \
    lighting_debug.cpp \
    profiler.cpp \
    profilerdialog.cpp \
    scriptables.cpp \
    materials.cpp \
    translations.cpp \
    sectormap.cpp \
    navigationmesh.cpp \
    models.cpp \
    imageuploader.cpp \
    selectioncircle.cpp \
    savegames.cpp \
    skeleton.cpp \
    animation.cpp \
    bindingpose.cpp \
    animatedmeshstate.cpp \
    modelfilereader.cpp \
    modelfilechunks.cpp \
    fogofwar.cpp \
    modelviewer.cpp \
    charactervault.cpp
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
    lighting.h \
    lighting_debug.h \
    drawhelper.h \
    profiler.h \
    profilerdialog.h \
    scriptables.h \
    materials.h \
    translations.h \
    sectormap.h \
    navigationmesh.h \
    vertexbufferobject.h \
    models.h \
    imageuploader.h \
    selectioncircle.h \
    savegames.h \
    skeleton.h \
    animation.h \
    bindingpose.h \
    animatedmeshstate.h \
    modelfilereader.h \
    modelfilechunks.h \
    gamemath_streams.h \
    fogofwar.h \
    modelviewer.h \
    charactervault.h
OTHER_FILES += \
    resources/schema/materialfile.xsd \
    resources/materials/map_material.xml \
    resources/materials/box_material.xml \
    resources/materials/light_material.xml \
    resources/materials/sprite_material.xml \
    resources/materials/clipping_material.xml \
    resources/materials/missing_material.xml \
    resources/materials/shadow.vs \
    resources/materials/shadow.fs \
    resources/materials/bounds_debug.tga \
    resources/materials/light_debug.tga \
    resources/fonts/5inq_-_Handserif.ttf \
    resources/fonts/ArtNoveauDecadente.ttf \
    resources/fonts/Fontin-Bold.ttf \
    resources/fonts/Fontin-Italic.ttf \
    resources/fonts/Fontin-Regular.ttf \
    resources/fonts/Fontin-SmallCaps.ttf \
    ../bin/data/scripts/startup.js \
    ../bin/data/scripts/equipment.js \
    ../bin/data/scripts/animations.js \
    resources/materials/spheremap.vert \
    resources/materials/spheremap.frag \
    resources/materials/lighting.frag \
    resources/materials/lighting.vert \
    resources/materials/textureanim.frag \
    resources/materials/selection_material.xml \
    resources/materials/behindwalls_material.xml \
    ../bin/data/scripts/walking.js \
    ../bin/data/scripts/mobileInfo.js \
    resources/materials/shadow.vert \
    resources/materials/shadow.frag \
    resources/materials/fog_material.xml

INCLUDEPATH += .

PRECOMPILED_HEADER = stable.h

win32:LIBS += -lpsapi

include(../base.pri)

RESOURCES += \
    resources.qrc

FORMS += \
    profilerdialog.ui
