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
    /**
     * This is a rather odd function. In this case, it loads all mobiles from all maps on initial startup and
     * creates several data-structures for holding them.
     */
    Maps.load = function() {
        /*
          Automatically load available zones. This should probably be refactored into some sort of "campaign" / "module"
          structure, which means maps only get loaded in module packages.
          */
        var mapIdMapping = readJson('legacy_maps.js');

        for (var mapId in mapIdMapping) {
            var filename = mapIdMapping[mapId];

            var map = new Map(mapId, filename);
            maps.push(map);
            print("Loaded map " + map.name);
        }
    };

    Maps.getByLegacyId = function(mapId) {
        for (var i = 0; i < maps.length; ++i) {
            if (maps[i].id == mapId)
                return maps[i];
        }
        return null;
    };

    Maps.goToMap = function(map, position) {
        // TODO: Should we assert, that the map object is actually in Maps.maps?

        if (this.currentMap)
            this.currentMap.leaving(map, position);

        this.currentMap = map;

        if (map)
            map.entering(position);
    };

})();
