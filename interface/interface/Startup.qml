import Qt 4.6

/*
    This is the first file loaded by the engine.
*/

Loader {
    id: startupLoader

    // The loader will be fitted to the viewport since its the root item,
    // but items loaded by the loader itself should also fill the viewport.
    resizeMode: "SizeLoaderToItem"

    function startup() {
        console.log("Starting up");
        source = "MainMenu.qml";
    }

    Component.onCompleted: startup();
}
