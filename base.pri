
CONFIG += debug_and_release
DESTDIR += ../bin/

CONFIG(debug, debug|release) {
 TARGET = $$join(TARGET,,,_d)
 CONFIG += warn_on
} else {
 CONFIG += warn_off
}

# All libraries reside in the target dir
LIBS += -L../bin/

# Add game libraries
contains(TEMPLE_LIBS,qt3d) {
    INCLUDEPATH += ../qt3d/math3d/ ../qt3d/enablers/
    CONFIG(debug, debug|release) {
        LIBS += -lqt3d_d
    } else {
        LIBS += -lqt3d
    }
}

contains(TEMPLE_LIBS,game) {
    INCLUDEPATH += ../game/include
    CONFIG(debug, debug|release) {
        LIBS += -lgame_d
    } else {
        LIBS += -lgame
    }
}
