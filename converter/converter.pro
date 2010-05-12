
TEMPLATE = app
CONFIG -= gui
CONFIG += console

TARGET = converter

QT += xml opengl xmlpatterns

TEMPLE_LIBS += troikaformats qt3d minizip

SOURCES += converter.cpp \
    collada.cpp \
    materialconverter.cpp \
    interfaceconverter.cpp \
    modelwriter.cpp \
    exclusions.cpp \
    mapconverter.cpp

HEADERS += \
    util.h \
    collada.h \
    converter.h \
    materialconverter.h \
    interfaceconverter.h \
    modelwriter.h \
    exclusions.h \
    mapconverter.h

include(../base.pri)
include(../3rdparty/jpeg-8a/jpeg-8a.pri)

RESOURCES += \
    resources.qrc

OTHER_FILES += exclusions.txt \
    material_template.xml
