var StartupListeners = {

    listeners: [],

    add: function(callback, provides, dependsOn) {
        if (!dependsOn)
            dependsOn = [];

        this.listeners.push({
            provides: provides,
            dependsOn: dependsOn,
            callback: callback
        });
    },

    call: function() {

        var lastLength = this.listeners.length;
        var started = [];

        // TODO: Verify that every dependency actually exists.

        while (this.listeners.length != 0) {
            this.listeners = this.listeners.filter(function(listener) {
                // Check for dependencies
                var dependenciesMet = listener.dependsOn.every(function (dependency) {
                    return started.indexOf(dependency) != -1;
                });

                if (!dependenciesMet) {
                    print("Deferring initialization of component " + listener.provides + " due to unmet dependencies.");
                    return true;
                }

                listener.callback.call();
                if (listener.provides) {
                    started.push(listener.provides);
                }

                return false;
            });

            if (this.listeners.length == lastLength) {
                throw "Would enter infinite startup loop due to unsatisfied dependencies or a dependency loop!";
            }
        }

    }

};

var prototypes;

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
