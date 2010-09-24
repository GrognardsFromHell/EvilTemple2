#-------------------------------------------------
#
# Project created by QtCreator 2010-09-17T23:08:13
#
#-------------------------------------------------

QT       += script

TARGET = conversion
TEMPLATE = lib

DEFINES += CONVERSION_LIBRARY

QT += xml opengl xmlpatterns
CONFIG += qaxcontainer precompile_header

TEMPLE_LIBS += troikaformats qt3d jpeg qjson glew python

HEADERS += \
    include/conversion/util.h \
    include/conversion/converter.h \
    include/conversion/global.h \
    include/conversion/materialconverter.h \
    include/conversion/modelwriter.h \
    include/conversion/exclusions.h \
    include/conversion/mapconverter.h \
    include/conversion/basepathfinder.h \
    include/conversion/prototypeconverter.h \
    include/conversion/pathnodeconverter.h \
    include/conversion/conversiontask.h \
    include/conversion/convertmapstask.h \
    include/conversion/convertscriptstask.h \
    include/conversion/converttranslationstask.h \
    include/conversion/convertparticlesystemstask.h \
    include/conversion/convertinterfacetask.h \
    include/conversion/convertsoundstask.h \
    include/conversion/convertmodelstask.h \
    include/conversion/pythonconverter.h \
    include/conversion/mapareamapping.h \
    include/conversion/convertmoviestask.h \
    include/conversion/qdirvfshandler.h \
    include/conversion/converthairtask.h \
    include/conversion/sectorconverter.h \
    include/conversion/clippingmeshconverter.h

HEADERS += src/stable.h

SOURCES += src/converter.cpp \
    src/materialconverter.cpp \
    src/modelwriter.cpp \
    src/exclusions.cpp \
    src/mapconverter.cpp \
    src/prototypeconverter.cpp \
    src/pathnodeconverter.cpp \
    src/conversiontask.cpp \
    src/convertmapstask.cpp \
    src/convertscriptstask.cpp \
    src/converttranslationstask.cpp \
    src/convertparticlesystemstask.cpp \
    src/convertinterfacetask.cpp \
    src/convertsoundstask.cpp \
    src/convertmodelstask.cpp \
    src/pythonconverter.cpp \
    src/mapareamapping.cpp \
    src/convertmoviestask.cpp \
    src/qdirvfshandler.cpp \
    src/converthairtask.cpp \
    src/sectorconverter.cpp \
    src/clippingmeshconverter.cpp

INCLUDEPATH += include/

win32:SOURCES += src/basepathfinder_win32.cpp
else:SOURCES += src/basepathfinder.cpp

win32:LIBS += -ladvapi32

include(../base.pri)
include(../3rdparty/game-math/game-math.pri)

RESOURCES += resources/resources.qrc

OTHER_FILES += resources/exclusions.txt \
    resources/material_template.xml \
    resources/shadow_caster.txt \
    resources/particlefiles.txt \
    resources/converter.js \
    resources/map_exclusions.txt \
    resources/map_area_mapping.txt \
    conversion.pri

PRECOMPILED_HEADER = src/stable.h

include(../common/common.pri)
