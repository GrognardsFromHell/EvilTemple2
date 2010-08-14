var WorldMapUi = {};

(function() {

    /**
     * These define the connection that each path establishes.
     */
    var pathConnections = {
        0: ['hommlet', 'emridy_meadows'],
        1: ['hommlet', 'moathouse'],
        2: ['hommlet', 'moathouse_secret_exit'],
        3: ['hommlet', 'nulb'],
        4: ['hommlet', 'imeryds_run'],
        5: ['hommlet', 'ogre_cave'],
        6: ['hommlet', 'temple'],
        7: ['hommlet', 'temple_secret_exit'],
        8: ['hommlet', 'deklo_grove'],
        9: ['emridy_meadows', 'deklo_grove'],
        10: ['deklo_grove', 'moathouse'],
        11: ['deklo_grove', 'moathouse_secret_exit'],
        12: ['deklo_grove', 'nulb'],
        13: ['deklo_grove', 'imeryds_run'],
        14: ['deklo_grove', 'ogre_cave'],
        15: ['deklo_grove', 'temple'],
        16: ['deklo_grove', 'temple_secret_exit'],
        17: ['emridy_meadows', 'temple_secret_exit'],
        18: ['moathouse', 'temple_secret_exit'],
        19: ['moathouse_secret_exit', 'temple_secret_exit'],
        20: ['imeryds_run', 'temple_secret_exit'],
        21: ['nulb', 'temple_secret_exit'],
        22: ['ogre_cave', 'temple_secret_exit'],
        23: ['temple', 'temple_secret_exit'],
        24: ['emridy_meadows', 'temple'],
        25: ['moathouse', 'temple'],
        26: ['moathouse_secret_exit', 'temple'],
        27: ['ogre_cave', 'temple'],
        28: ['nulb', 'temple'],
        29: ['temple', 'imeryds_run'],
        30: ['emridy_meadows', 'ogre_cave'],
        31: ['ogre_cave', 'moathouse'],
        32: ['ogre_cave', 'moathouse_secret_exit'],
        33: ['ogre_cave', 'nulb'],
        34: ['ogre_cave', 'imeryds_run'],
        35: ['emridy_meadows', 'imeryds_run'],
        36: ['moathouse', 'imeryds_run'],
        37: ['moathouse_secret_exit', 'imeryds_run'],
        38: ['nulb', 'imeryds_run'],
        39: ['emridy_meadows', 'nulb'],
        40: ['moathouse', 'nulb'],
        41: ['moathouse_secret_exit', 'nulb'],
        42: ['emridy_meadows', 'moathouse_secret_exit'],
        43: ['moathouse', 'moathouse_secret_exit'],
        44: ['emridy_meadows', 'moathouse']
    };

    var worldMapPaths = null;

    var worldMapDialog = null;

    var newAreaSinceLastOpened = false;

    function loadPaths() {
        if (worldMapPaths == null) {
            worldMapPaths = readJson('worldmapPaths.js');
        }
    }

    StartupListeners.add(function() {
        WorldMap.addMarkedNewAreaListener(function() {
            if (worldMapDialog) {
                // Play sound immediately & update ui
                worldMapDialog.knownAreas = WorldMap.getMarkedAreas();
                playNewAreaSound();
            } else {
                newAreaSinceLastOpened = true;
            }
        });
    });

    WorldMapUi.show = function() {
        if (worldMapDialog)
            return;

        worldMapDialog = gameView.addGuiItem("interface/WorldMap.qml");
        worldMapDialog.closeClicked.connect(WorldMapUi.close);
        worldMapDialog.knownAreas = WorldMap.getMarkedAreas();
        worldMapDialog.currentArea = Maps.currentMap.area;

        if (newAreaSinceLastOpened) {
            playNewAreaSound();
            newAreaSinceLastOpened = false;
        }

        loadPaths();
        worldMapDialog.travelRequested.connect(function (to) {
            print("Worldmap requested travel to " + to);
            var from = Maps.currentMap.area;

            // Try to find a connection.
            for (var pathId in pathConnections) {
                var connection = pathConnections[pathId];

                if (connection[0] == from && connection[1] == to) {
                    worldMapDialog.travelPath(worldMapPaths[pathId]);
                    return;
                } else if (connection[1] == from && connection[0] == to) {
                    // TODO: reverse path
                    worldMapDialog.travelPath(worldMapPaths[pathId]);
                    return;
                }
            }

            // No path could be found. What to do now?
            print("No path found for connection " + from + " -> " + to);
        });
        worldMapDialog.travelFinished.connect(function() {
            print("Travelling is finished. Arrived at new destination.");
        });
    };

    WorldMapUi.close = function() {
        if (!worldMapDialog)
            return;

        worldMapDialog.deleteLater();
        worldMapDialog = null;
    };

    function playNewAreaSound() {
        gameView.playUiSound('sound/worldmap_location.wav');
    }

})();
