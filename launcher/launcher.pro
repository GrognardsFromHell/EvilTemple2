
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

win32:RC_FILE = icon.rc

include(../base.pri)

win32-msvc2008:include(../3rdparty/google-breakpad.pri)
win32-msvc2010:include(../3rdparty/google-breakpad.pri)

include(../common/common.pri)

OTHER_FILES += \
    icon.rc \
    application.ico
