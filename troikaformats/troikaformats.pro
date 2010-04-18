
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
    material.h \
    modelsource.h \
    skeleton.h \
    glext.h \
    prototypes.h \
    util.h \
    constants.h \
    materials.h \
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
    material.cpp \
    skeleton.cpp \
    prototypes.cpp \
    materials.cpp \
    zonebackgroundmap.cpp \
    zonetemplate.cpp \
    zonetemplates.cpp

TEMPLE_LIBS += qt3d

PRECOMPILED_HEADER = stable.h

include(../base.pri)
