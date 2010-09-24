
INCLUDEPATH += $${PWD} $${PWD}/../3rdparty/minizip/ $${PWD}/../3rdparty/SFMT-src-1.3.3/SFMT

include($$PWD/../buildroot.pri)

LIBS *= -L$${BUILD_ROOT}bin/

CONFIG(debug, debug|release) {
    LIBS += -lcommon_d
} else {
    LIBS += -lcommon
}
