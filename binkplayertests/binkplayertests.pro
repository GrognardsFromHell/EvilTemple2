
TEMPLATE = app

TEMPLE_LIBS += binkplayer game

SOURCES += \
    main.cpp \
    dialog.cpp

QT += opengl
CONFIG += console

include(../base.pri)

HEADERS += \
    dialog.h \
    wglext.h

FORMS += \
    dialog.ui
