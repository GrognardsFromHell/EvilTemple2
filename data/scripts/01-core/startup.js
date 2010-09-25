
/**
 * This function is called by the game engine when all scripts have been loaded.
 */
function startup() {
    print("Calling startup hooks.");
    StartupListeners.call();

    print("Loading jump points...");
    jumppoints = eval('(' + readFile('jumppoints.js') + ')');
    loadInventoryIcons();
    sounds = eval('(' + readFile('sound/sounds.js') + ')');

    print("Loading subsystems.");
    LegacyScripts.load();
    LegacyDialog.load();
    try {
        Maps.load();
    } catch (e) {
        throw "Error Loading Maps: " + e;
    }

    // Accept clicks from the game view
    setupWorldClickHandler();

    print("Showing main menu.");
    MainMenuUi.show();
}
