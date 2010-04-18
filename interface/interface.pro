
TEMPLATE = lib

OTHER_FILES += interface/MainMenu.qml \
    interface/MainMenuButton.qml \
    interface/Console.qml \
    interface/Startup.qml \
    interface/LoadGame.qml

copyQml.target = copyQml
copyQml.commands = copyQml.bat

QMAKE_EXTRA_TARGETS += copyQml

POST_TARGETDEPS += $$copyQml.target

SOURCES += \
    dummy.cpp
