
TEMPLATE = app

TARGET = converter

QT += xml opengl xmlpatterns script
CONFIG += qaxcontainer

TEMPLE_LIBS += troikaformats qt3d minizip jpeg qjson

win32:RC_FILE = icon.rc

SOURCES += converter.cpp \
    collada.cpp \
    materialconverter.cpp \
    interfaceconverter.cpp \
    modelwriter.cpp \
    exclusions.cpp \
    mapconverter.cpp \
    prototypeconverter.cpp \
    converterwizard.cpp \
    choosedirectorypage.cpp \
    conversionpage.cpp \
    pathnodeconverter.cpp

HEADERS += \
    util.h \
    collada.h \
    converter.h \
    materialconverter.h \
    interfaceconverter.h \
    modelwriter.h \
    exclusions.h \
    mapconverter.h \
    basepathfinder.h \
    prototypeconverter.h \
    stable.h \
    converterwizard.h \
    choosedirectorypage.h \
    conversionpage.h \
    pathnodeconverter.h

win32:SOURCES += basepathfinder_win32.cpp
else:SOURCES += basepathfinder.cpp

win32:LIBS += -ladvapi32

include(../base.pri)
include(../3rdparty/game-math/game-math.pri)

RESOURCES += \
    resources.qrc

OTHER_FILES += exclusions.txt \
    material_template.xml \
    shadow_caster.txt \
    particlefiles.txt \
    scripts/converter.js \
    icon.rc

PRECOMPILED_HEADER = stable.h

FORMS += \
    choosedirectorypage.ui \
    conversionpage.ui
