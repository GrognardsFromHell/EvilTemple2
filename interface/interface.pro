
TEMPLATE = lib

OTHER_FILES += interface/MainMenu.qml \
    interface/MainMenuButton.qml \
    interface/Console.qml \
    interface/Startup.qml \
    interface/LoadGame.qml \
    copyQml.sh \
    copyQml.bat \
    interface/MobileInfo.qml

copyQml.target = copyQml
win32:copyQml.commands = copyQml.bat
unix:copyQml.commands = sh copyQml.sh

QMAKE_EXTRA_TARGETS += copyQml

POST_TARGETDEPS += $$copyQml.target

SOURCES += \
    dummy.cpp
