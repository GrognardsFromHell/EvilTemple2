TEMPLATE = lib
isEmpty(QT_MAJOR_VERSION) {
   VERSION=4.6.0
} else {
   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
}
CONFIG += qt plugin

win32|mac:!wince*:!win32-msvc:!macx-xcode:CONFIG += debug_and_release
TARGET = $$qtLibraryTarget($$TARGET)
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

wince*:LIBS += $$QMAKE_LIBS_GUI

symbian: {
    TARGET.EPOCALLOWDLLDATA=1
    contains(QT_EDITION, OpenSource) {
        TARGET.CAPABILITY = LocalServices NetworkServices ReadUserData UserEnvironment WriteUserData
    } else {
        TARGET.CAPABILITY = All -Tcb        
    }
}
