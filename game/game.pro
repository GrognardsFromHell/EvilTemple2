# -------------------------------------------------
# Project created by QtCreator 2009-12-13T19:44:20
# -------------------------------------------------
QT += network \
    opengl \
    script \
    scripttools \
    webkit \
    xml \
    xmlpatterns \
    phonon \
    multimedia \
    testlib
TARGET = EvilTemple
TEMPLATE = app
SOURCES += src/io/tgaplugin.cpp \
    src/main.cpp \
    src/ui/mainwindow.cpp \
    src/ui/gamegraphicsscene.cpp \
    src/io/troikaarchive.cpp \
    src/game.cpp \
    src/io/virtualfilesystem.cpp \
    src/ui/cursors.cpp \
    src/model.cpp \
    src/material.cpp \
    src/materials.cpp \
    src/io/skmreader.cpp \
    src/ui/gamegraphicsview.cpp \
    src/camera.cpp \
    src/campaign/zone.cpp \
    src/zonetemplate.cpp \
    src/zonetemplates.cpp \
    src/campaign/campaign.cpp \
    src/io/messagefile.cpp \
    src/io/zonetemplatereader.cpp \
    src/zonebackgroundmap.cpp \
    src/geometrymeshobject.cpp \
    src/io/objectfilereader.cpp \
    src/prototypes.cpp \
    src/io/dagreader.cpp \
    src/clippinggeometryobject.cpp \
    src/skeleton.cpp \
    src/ui/meshdialog.cpp \
    src/models.cpp \
    src/animationcontroller.cpp \
    src/ui/consolewidget.cpp \
    src/scriptengine.cpp
HEADERS += include/io/tgaplugin.h \
    include/ui/mainwindow.h \
    include/ui/gamegraphicsscene.h \
    include/ui/cursors.h \
    include/io/troikaarchive.h \
    include/game.h \
    include/io/virtualfilesystem.h \
    include/stable.h \
    include/io/basepathfinder.h \
    include/model.h \
    include/material.h \
    include/materials.h \
    include/io/skmreader.h \
    include/ui/gamegraphicsview.h \
    include/camera.h \
    include/campaign/zone.h \
    include/zonetemplate.h \
    include/zonetemplates.h \
    include/campaign/campaign.h \
    include/io/messagefile.h \
    include/io/zonetemplatereader.h \
    include/zonebackgroundmap.h \
    include/geometrymeshobject.h \
    include/modelsource.h \
    include/util.h \
    include/io/objectfilereader.h \
    include/prototypes.h \
    include/io/dagreader.h \
    include/clippinggeometryobject.h \
    include/skeleton.h \
    include/ui/meshdialog.h \
    include/models.h \
    include/animationcontroller.h \
    include/ui/consolewidget.h \
    include/scriptengine.h
OTHER_FILES += config.xsd \
    resources/console/style.css
INCLUDEPATH += include/
PRECOMPILED_HEADER = include/stable.h
include(qt3d/qt3d.pri)

# Win32 specific code
win32:SOURCES += src/io/basepathfinder_win32.cpp
else:SOURCES += src/io/basepathfinder.cpp
win32-g++:message("Enabling extended warnings.")
win32:LIBS += -ladvapi32 -lpsapi
FORMS += meshdialog.ui
RESOURCES += resources/resources.qrc
