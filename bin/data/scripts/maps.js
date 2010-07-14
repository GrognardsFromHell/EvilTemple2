/**
 * Manages available zones.
 */
var Maps = {
    /**
     * The currently loaded map object.
     */
    currentMap: null,

    /**
     * All map objects in the current campaign.
     */
    maps: []
};

(function() {

    /*
      Automatically load available zones. This should probably be refactored into some sort of "campaign" / "module"
      structure, which means maps only get loaded in module packages.
      */
    var mapIdMapping = eval('(' + readFile('legacy_maps.js') + ')');

    /**
     * Gets an array of all available map ids.
     */
    Maps.maps = function() {
        var result = [];
        for (var k in maps)
            result.push(k);
        return result;
    };

    Maps.goToMap = function(map, position) {
        // TODO: Should we assert, that the map object is actually in Maps.maps?

        var newMap = Maps.maps[id];

        if (currentMap)
            currentMap.leaving();

        currentMap = map;

        if (currentMap) {
            // TODO: Position the party on the map

            currentMap.entering(position);
        }
    };

})();
