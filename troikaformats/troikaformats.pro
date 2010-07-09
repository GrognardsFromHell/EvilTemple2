
TEMPLATE = lib
TARGET = troikaformats

QT += opengl

CONFIG += precompile_header

DEFINES += TROIKAFORMATS_LIBRARY

HEADERS += dagreader.h \
    messagefile.h \
    objectfilereader.h \
    skmreader.h \
    troikaarchive.h \
    virtualfilesystem.h \
    zonetemplatereader.h \
    model.h \
    troika_material.h \
    modelsource.h \
    skeleton.h \
    prototypes.h \
    util.h \
    constants.h \
    troika_materials.h \
    zonetemplate.h \
    zonebackgroundmap.h \
    zonetemplates.h \
    stable.h \
    troikaformatsglobal.h

SOURCES += dagreader.cpp \
    messagefile.cpp \
    objectfilereader.cpp \
    skmreader.cpp \
    troikaarchive.cpp \
    virtualfilesystem.cpp \
    zonetemplatereader.cpp \
    model.cpp \
    troika_material.cpp \
    skeleton.cpp \
    prototypes.cpp \
    troika_materials.cpp \
    zonebackgroundmap.cpp \
    zonetemplate.cpp \
    zonetemplates.cpp

TEMPLE_LIBS += qt3d glew

include(../3rdparty/game-math/game-math.pri)

PRECOMPILED_HEADER = stable.h

include(../base.pri)

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    prototypes_flags.txt
