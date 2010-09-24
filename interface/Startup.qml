import Qt 4.7

/*
    This is the first file loaded by the engine.
*/

Loader {
    id: startupLoader

    function startup() {
        console.log("Starting up");
        source = "MainMenu.qml";
    }

    Component.onCompleted: startup();
}
