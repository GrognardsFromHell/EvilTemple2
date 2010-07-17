
var WorldMapUi = {};

(function() {

    var worldMapDialog = null;

    var newAreaSinceLastOpened = false;

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
