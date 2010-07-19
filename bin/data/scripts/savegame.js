/**
 * This object manages the current savegames.
 */
var SaveGames = {};

(function() {

    var loadedListeners = new ListenerQueue;
    var savedListeners = new ListenerQueue;
    var savingListeners = new ListenerQueue;
    var loadingListeners = new ListenerQueue;

    function quicksave() {
        print("Triggering quicksave.");
        SaveGames.save('Quicksave', 'quicksave');
    }

    /**
     * Creates a new savegame.
     * @param name A name for the new savegame.
     */
    SaveGames.save = function(name, id) {
        var screenshot = gameView.takeScreenshot();
        var record;

        try {
            var payload = {
                'viewCenter': gameView.worldCenter()
            };

            /*
             * Notify all saving listeners to actually populate the save payload.
             */
            savingListeners.notify(payload);

            if (id)
                record = savegames.save(id, name, screenshot, JSON.stringify(payload));
            else
                record = savegames.save(name, screenshot, JSON.stringify(payload));
        } finally {
            gameView.deleteScreenshot(screenshot);
        }

        /*
         * Notify listeners that a savegame has been created/saved.
         */
        savedListeners.notify(record);
    };

    /**
     * Loads a save-game by id.
     *
     * @param id The unique id of the savegame to load.
     */
    SaveGames.load = function(id) {
        print("Loading save-game " + id);

        var payload = JSON.parse(savegames.load(id));

        gameView.centerOnWorld(payload.viewCenter);

        /*
         Notify listeners that a savegame is being loaded.
         */
        loadingListeners.notify(payload);

        /**
         * Once all data has been loaded, notify the second tier of listeners.
         * This allows listeners to assume that all state has been restored.
         */
        loadedListeners.notify(payload);
    };

    SaveGames.addSavingListener = function(callback, thisObject) {
        savingListeners.append(callback, thisObject);
    };

    SaveGames.addLoadingListener = function(callback, thisObject) {
        loadingListeners.append(callback, thisObject);
    };

    SaveGames.addLoadedListener = function(callback, thisObject) {
        loadedListeners.append(callback, thisObject);
    };

    SaveGames.addSavedListener = function(callback, thisObject) {
        savedListeners.append(callback, thisObject);
    };

    StartupListeners.add(function() {
        Shortcuts.register(Keys.F5, quicksave);
    });

})();
