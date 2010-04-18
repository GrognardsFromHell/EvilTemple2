
TEMPLATE = app

QT += testlib
CONFIG  += qtestlib

include(../../base.pri)
DESTDIR = ../../bin

HEADERS += \
    testzipwriter.h

SOURCES += \
    testzipwriter.cpp

LIBS += -L../../bin
INCLUDEPATH += ../minizip
CONFIG(debug, debug|release) {
    LIBS += -lminizip_d
} else {
    LIBS += -lminizip
}
