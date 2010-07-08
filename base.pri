
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

contains(TEMPLE_LIBS,game) {
    INCLUDEPATH += ../game
    CONFIG(debug, debug|release) {
        LIBS += -lgame_d
    } else {
        LIBS += -lgame
    }
}

# It's possible we should instead use the system libjpeg here on unix
contains(TEMPLE_LIBS,jpeg) {
    INCLUDEPATH += $${PWD}/3rdparty/libjpeg-turbo/include
    win32:LIBS += $${PWD}/3rdparty/libjpeg-turbo/lib/turbojpeg.lib
    else:LIBS += -L$${PWD}/bin/ -lturbojpeg
}

contains(TEMPLE_LIBS,glew) {
    # GLEW is not a system library on windows,
    # so use the packaged one
    win32 {
        INCLUDEPATH += $${PWD}/3rdparty/glew-1.5.4/include
        LIBS += $${PWD}/3rdparty/glew-1.5.4/lib/glew32s.lib
        LIBS += -lopengl32
        DEFINES += GLEW_STATIC
    } else {
        LIBS += -lGLEW
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

contains(TEMPLE_LIBS,libavcodec) {
    win32:INCLUDEPATH += $${PWD}/3rdparty/libavcodec/include
    win32:LIBS += -L$${PWD}/3rdparty/libavcodec/lib

    win32-msvc2008:INCLUDEPATH += $${PWD}/3rdparty/libavcodec/msvc
    win32-msvc2010:INCLUDEPATH += $${PWD}/3rdparty/libavcodec/msvc

    LIBS += -lavcodec -lavformat -lavutil -lswscale
}

contains(TEMPLE_LIBS,minizip) {
    INCLUDEPATH += ../3rdparty/minizip
    CONFIG(debug, debug|release) {
        LIBS += -lminizip_d
    } else {
        LIBS += -lminizip
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

contains(TEMPLE_LIBS,qjson) {
    INCLUDEPATH += $${PWD}/3rdparty/qjson/src/
    LIBS += -lqjson
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

# Enable to get PDB for release builds
#win32:QMAKE_CXXFLAGS_RELEASE += -Zi
#win32:QMAKE_LFLAGS_RELEASE += /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF
