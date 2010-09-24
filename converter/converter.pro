
TEMPLATE = app

TARGET = converter

QT += xml opengl xmlpatterns script
CONFIG += qaxcontainer

TEMPLE_LIBS += troikaformats qt3d jpeg qjson glew

win32:RC_FILE = icon.rc

SOURCES += converterwizard.cpp \
    choosedirectorypage.cpp \
    conversionpage.cpp

HEADERS += converterwizard.h \
    choosedirectorypage.h \
    conversionpage.h

win32:LIBS += -ladvapi32

include(../base.pri)
include(../3rdparty/game-math/game-math.pri)

include(../conversion/conversion.pri)

RESOURCES += resources.qrc \
    resources.qrc

OTHER_FILES += icon.rc

FORMS += choosedirectorypage.ui \
    conversionpage.ui

include(../common/common.pri)
