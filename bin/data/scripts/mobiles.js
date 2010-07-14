/**
 * In a fully populated save-game, all mobiles from all maps will have to be in-memory, since
 * they all can have standpoints on different maps. Instead of reloading them everytime a
 * savegame is loaded, we load all mobiles on game startup.
 */
var Mobiles = {};

(function() {

    var mobiles = [];

    /**
     * Iterates over all known maps and loads associated mobiles.
     */
    Mobiles.load = function() {
        /*Maps.getAvailableIds().forEach(function (mapId) {
            var mapFile = Maps.getFilename(mapId);
            var mapObj = eval('(' + readFile(mapFile) + ')');
            var mapMobiles = eval('(' + readFile(mapObj.mobiles) + ')');
            for (var i = 0; i < mapMobiles.length; ++i) {
                mobiles.push(mapMobiles[i]);
            }
        });*/

        print("Loaded " + mobiles.length + " mobiles in total.");
    };

})();
