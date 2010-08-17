
TEMPLATE = lib
CONFIG += qt plugin
QT += declarative

DESTDIR = lib
OBJECTS_DIR = tmp
MOC_DIR = tmp

OTHER_FILES += \
    README.txt \
    qmldir

HEADERS += \
    modelviewerplugin.h \
    modelviewer.h

SOURCES += \
    modelviewerplugin.cpp \
    modelviewer.cpp
