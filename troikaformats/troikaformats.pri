
include($${PWD}/../3rdparty/game-math/game-math.pri)

INCLUDEPATH += $${PWD}
CONFIG(debug, debug|release) {
    LIBS += -L$${PWD}/../bin
    LIBS += -ltroikaformats_d
} else {
    LIBS += -L$${PWD}/../bin
    LIBS += -ltroikaformats
}
