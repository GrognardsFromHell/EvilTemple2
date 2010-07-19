
/*
    Manages global boolean flags that are persisted to the current save game.
 */
var GlobalFlags = {};

(function () {

    var flags = {};

    /**
     * Checks whether a global flag is currently set.
     * @param id The id of the flag to check.
     */
    GlobalFlags.isSet = function(id) {
        return flags[id] !== undefined;
    };

    /**
     * Sets a global flag, regardless of its current value.
     * @param id The id of the flag to set.
     */
    GlobalFlags.set = function(id) {
        print("Setting flag " + id);
        flags[id] = true;
    };

    /**
     * Clears a global flag, whether it has been set before or not.
     * @param id The id of the global flag.
     */
    GlobalFlags.unset = function(id) {
        print("Unsetting flag " + id);
        delete flags[id];
    };

    function save(payload) {
        payload.globalFlags = flags;
    }

    function load(payload) {
        flags = payload.globalFlags;
    }

    StartupListeners.add(function() {
        SaveGames.addSavingListener(save);
        SaveGames.addLoadingListener(load);
    });

})();
