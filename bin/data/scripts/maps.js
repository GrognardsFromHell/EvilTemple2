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
    maps: [],

    /**
     * Stores maps by their id.
     */
    mapsById: {}
};

(function() {
    var legacyIdMapping = readJson('legacy_maps.js');

    /**
     * This is a rather odd function. In this case, it loads all mobiles from all maps on initial startup and
     * creates several data-structures for holding them.
     */
    Maps.load = function() {
        /*
         Automatically load available zones. This should probably be refactored into some sort of "campaign" / "module"
         structure, which means maps only get loaded in module packages.
         */
        for (var legacyId in legacyIdMapping) {
            var mapId = legacyIdMapping[legacyId];

            var map = new Map(mapId);
            this.maps.push(map);
            this.mapsById[mapId] = map;
            print("Loaded map " + map.name + ' (' + mapId + ')');
        }
    };

    Maps.getByLegacyId = function(legacyId) {
        var mapId = legacyIdMapping[legacyId];
        return Maps.mapsById[mapId];
    };

    /**
     * Searches through all maps to find a mobile with a given GUID.
     *
     * @param id The GUID.
     */
    Maps.findMobileById = function(id) {
        for (var i = 0; i < this.maps.length; ++i) {
            var result = this.maps[i].findMobileById(id);
            if (result)
                return result;
        }
        return result;
    };

    Maps.goToMap = function(map, position) {
        // TODO: Should we assert, that the map object is actually in Maps.maps?
        if (!this.mapsById[map.id]) {
            print("Trying to go to a map that is not registered.");
            return;
        }

        if (this.currentMap)
            this.currentMap.leaving(map, position);

        this.currentMap = map;

        if (map)
            map.entering(position);
    };

})();
