
TEMPLATE = app

TARGET = eviltemple

SOURCES += main.cpp

QT += opengl declarative

TEMPLE_LIBS += game qt3d

# A console is helpful for debugging
CONFIG(debug, debug|release) {
    CONFIG += console
}


CONFIG += console


include(../base.pri)
