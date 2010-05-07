
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
    exclusions.cpp

HEADERS += \
    util.h \
    collada.h \
    converter.h \
    materialconverter.h \
    interfaceconverter.h \
    modelwriter.h \
    exclusions.h

include(../base.pri)

RESOURCES += \
    resources.qrc

OTHER_FILES += exclusions.txt
