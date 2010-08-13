
var WorldMapUi = {};

(function() {

    var worldMapDialog = null;

    var newAreaSinceLastOpened = false;

    var worldMapPaths = null;

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
        worldMapDialog.travelRequested.connect(function (areaId) {
            print("Worldmap requested travel to " + areaId);
            worldMapDialog.travelPath(worldMapPaths[1]);
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
