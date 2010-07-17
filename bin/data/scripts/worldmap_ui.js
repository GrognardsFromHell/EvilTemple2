
var WorldMapUi = {};

(function() {

    var worldMapDialog = null;

    WorldMapUi.show = function() {
        if (worldMapDialog)
            return;

        worldMapDialog = gameView.addGuiItem("interface/WorldMap.qml");
        worldMapDialog.closeClicked.connect(WorldMapUi.close);
        worldMapDialog.knownAreas = WorldMap.getMarkedAreas();
        worldMapDialog.currentArea = Maps.currentMap.area;
    };

    WorldMapUi.close = function() {
        if (!worldMapDialog)
            return;

        worldMapDialog.deleteLater();
        worldMapDialog = null;
    };

})();
