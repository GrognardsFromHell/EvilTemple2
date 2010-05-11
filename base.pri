
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
    INCLUDEPATH += ../3rdparty/qt3d/math3d/ ../3rdparty/qt3d/enablers/
    CONFIG(debug, debug|release) {
        LIBS += -lqt3d_d
    } else {
        LIBS += -lqt3d
    }
}

contains(TEMPLE_LIBS,game-math) {
    include($${PWD}/3rdparty/game-math/game-math.pri);
}

contains(TEMPLE_LIBS,game) {
    INCLUDEPATH += ../game
    CONFIG(debug, debug|release) {
        LIBS += -lgame_d
    } else {
        LIBS += -lgame
    }
}

contains(TEMPLE_LIBS,troikaformats) {
    INCLUDEPATH += ../troikaformats
    CONFIG(debug, debug|release) {
        LIBS += -ltroikaformats_d
    } else {
        LIBS += -ltroikaformats
    }
}

contains(TEMPLE_LIBS,minizip) {
    INCLUDEPATH += ../3rdparty/minizip
    CONFIG(debug, debug|release) {
        LIBS += -lminizip_d
    } else {
        LIBS += -lminizip
    }
}

contains(TEMPLE_LIBS,model) {
    INCLUDEPATH += ../model
    CONFIG(debug, debug|release) {
        LIBS += -lmodel_d
    } else {
        LIBS += -lmodel
    }
}

contains(TEMPLE_LIBS,audioengine) {
    INCLUDEPATH += ../audioengine
    CONFIG(debug, debug|release) {
        LIBS += -laudioengine_d
    } else {
        LIBS += -laudioengine
    }
}

# Add game libraries
contains(TEMPLE_LIBS,binkplayer) {
    INCLUDEPATH += ../binkplayer/
    CONFIG(debug, debug|release) {
        LIBS += -lbinkplayer_d
    } else {
        LIBS += -lbinkplayer
    }
}

contains(TEMPLE_LIBS,openal) {    
    win32 {
        INCLUDEPATH += ../3rdparty/openal-1.1/include ../3rdparty/freealut-1.1.0/include/
        LIBS += ../3rdparty/openal-1.1/libs/Win32/OpenAL32.lib

        CONFIG(debug, debug|release) {
            LIBS += -lfreealut_d
        } else {
            LIBS += -lfreealut
        }
    } else {
        LIBS += -lopenal -lalut
    }
}


contains(TEMPLE_LIBS,eigen) {
    INCLUDEPATH += ../3rdparty/eigen/
}
