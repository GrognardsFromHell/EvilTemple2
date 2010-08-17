
CONFIG += debug_and_release
DESTDIR = $${PWD}/bin/

CONFIG(debug, debug|release) {
 TARGET = $$join(TARGET,,,_d)
 CONFIG += warn_on
} else {
 CONFIG += warn_off
}

# All libraries reside in the target dir
LIBS += -L$${PWD}/bin/

# Add game libraries
contains(TEMPLE_LIBS,qt3d) {
    INCLUDEPATH += ../3rdparty/qt3d/math3d/ ../3rdparty/qt3d/enablers/
    CONFIG(debug, debug|release) {
        LIBS += -lqt3d_d
    } else {
        LIBS += -lqt3d
    }
}

contains(TEMPLE_LIBS,python) {
    win32 {
        INCLUDEPATH += C:/python26/include/
        LIBS += -LC:/python26/libs/ -lpython26
    }
    unix {
        # This probably needs proper detection code.
        LIBS += -lpython2.5
        INCLUDEPATH += /usr/include/python2.5
    }
}

contains(TEMPLE_LIBS,game) {
    INCLUDEPATH += $${PWD}/game
    CONFIG(debug, debug|release) {
        LIBS += -lgame_d
    } else {
        LIBS += -lgame
    }
}

contains(TEMPLE_LIBS,sfmt) {
    INCLUDEPATH += $${PWD}/3rdparty/SFMT-src-1.3.3
    SOURCES += $${PWD}/3rdparty/SFMT-src-1.3.3/SFMT.c
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
    INCLUDEPATH += $${PWD}/troikaformats
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

contains(TEMPLE_LIBS,audioengine) {
    INCLUDEPATH += ../audioengine
    CONFIG(debug, debug|release) {
        LIBS += -laudioengine_d
    } else {
        LIBS += -laudioengine
    }

    unix:TEMPLE_LIBS += libavcodec
}

contains(TEMPLE_LIBS,libavcodec) {
    win32:INCLUDEPATH += $${PWD}/3rdparty/libavcodec-minimal/include
    win32:LIBS += -L$${PWD}/3rdparty/libavcodec-minimal

    win32-msvc2008:INCLUDEPATH += $${PWD}/3rdparty/libavcodec-minimal/msvc
    win32-msvc2010:INCLUDEPATH += $${PWD}/3rdparty/libavcodec-minimal/msvc

    unix {
        INCLUDEPATH += $${PWD}/3rdparty/ffmpeg/include/
        LIBS += -L$${PWD}/3rdparty/ffmpeg/lib/ -lavcodec -lavformat -lavutil -lswscale
    }

    win32 {
        LIBS += -lavcodec-52 -lavformat-52 -lavutil-50 -lswscale-0
    }
}

contains(TEMPLE_LIBS,qjson) {
    INCLUDEPATH += $${PWD}/3rdparty/qjson/src/
    CONFIG(debug, debug|release) {
        LIBS += -lqjson_d
    } else {
        LIBS += -lqjson
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

    unix:TEMPLE_LIBS += libavcodec
}

contains(TEMPLE_LIBS,openal) {
    win32 {
        INCLUDEPATH += ../3rdparty/openal-1.1/include
        LIBS += ../3rdparty/openal-1.1/libs/Win32/OpenAL32.lib
    } else {
        LIBS += -lopenal
    }
}

# Enable to get PDB for release builds
win32-msvc2008:QMAKE_CXXFLAGS_RELEASE += -Zi
win32-msvc2008:QMAKE_LFLAGS_RELEASE += /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF
win32-msvc2010:QMAKE_CXXFLAGS_RELEASE += -Zi
win32-msvc2010:QMAKE_LFLAGS_RELEASE += /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF

OTHER_FILES += \
    ../bin/data/scripts/mobileInfo.js
