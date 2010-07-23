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

    function save(payload) {
        // Save the current map
        if (Maps.currentMap)
            payload.currentMap = Maps.currentMap.id;
        else
            payload.currentMap = null;

        payload.maps = [];

        // Store the mobiles of all maps
        Maps.maps.forEach(function (map) {
            payload.maps.push(map.persistState());
        });
    }

    function load(payload) {
        Maps.maps = payload.maps;
        Maps.mapsById = {};

        print("Loading " + Maps.maps.length + " maps.");

        Maps.maps.forEach(function (map) {
            print("Loading map " + map.id + " from savegame.");
            Maps.mapsById[map.id] = map;
            map.__proto__ = Map.prototype;

            // Reconnect all mobiles back to their prototypes
            map.mobiles.forEach(connectToPrototype);
            map.mobiles.forEach(function(mobile) {
                mobile.map = map;
            });
        });

        // Reload the current map
        if (payload.currentMap) {
            Maps.currentMap = Maps.mapsById[payload.currentMap];

            if (!Maps.currentMap) {
                print("The current map used by the savegame is no longer available!");
                return;
            }

            Maps.currentMap.reload();

            UtilityBarUi.update();
        }
    }

    function updateLighting() {
        // Update the lighting model
        if (Maps.currentMap) {
            Maps.currentMap.updateLighting();
        }
    }

    function updateDayNight(oldHour) {
        if (Maps.currentMap) {
            Maps.currentMap.updateDayNight(oldHour);
        }
    }

    StartupListeners.add(function() {
        SaveGames.addSavingListener(save);
        SaveGames.addLoadingListener(load);
        GameTime.addTimeChangedListener(updateLighting);
        GameTime.addHourChangedListener(updateDayNight);
    });

})();
