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

    var mouseClickListener = new ListenerQueue;
    var mouseDoubleClickListener = new ListenerQueue;
    var mouseMoveListener = new ListenerQueue;
    var mouseEnterListener = new ListenerQueue;
    var mouseLeaveListener = new ListenerQueue;

    Maps.addMouseClickListener = function(listener, thisObject) {
        mouseClickListener.append(listener, thisObject);
    };

    Maps.removeMouseClickListener = function(listener, thisObject) {
        mouseClickListener.remove(listener, thisObject);
    };

    Maps.addMouseDoubleClickListener = function(listener, thisObject) {
        mouseDoubleClickListener.append(listener, thisObject);
    };

    Maps.removeMouseDoubleClickListener = function(listener, thisObject) {
        mouseDoubleClickListener.remove(listener, thisObject);
    };

    Maps.addMouseMoveListener = function(listener, thisObject) {
        mouseMoveListener.append(listener, thisObject);
    };

    Maps.removeMouseMoveListener = function(listener, thisObject) {
        mouseMoveListener.remove(listener, thisObject);
    };

    Maps.addMouseEnterListener = function(listener, thisObject) {
        mouseEnterListener.append(listener, thisObject);
    };

    Maps.removeMouseEnterListener  = function(listener, thisObject) {
        mouseEnterListener.remove(listener, thisObject);
    };

    Maps.addMouseLeaveListener = function(listener, thisObject) {
        mouseLeaveListener.append(listener, thisObject);
    };

    Maps.removeMouseLeaveListener = function(listener, thisObject) {
        mouseLeaveListener.remove(listener, thisObject);
    };

    Maps.mouseReleased = function(event) {
        var worldPos = gameView.worldFromScreen(event.x, event.y);
        mouseClickListener.notify(event, worldPos);
    };

    Maps.mouseDoubleClicked = function(event) {
        var worldPos = gameView.worldFromScreen(event.x, event.y);
        mouseDoubleClickListener.notify(event, worldPos);
    };

    Maps.mouseMoved = function(event) {
        var worldPos = gameView.worldFromScreen(event.x, event.y);
        mouseMoveListener.notify(event, worldPos);
    };

    Maps.mouseEnter = function(event) {
        var worldPos = gameView.worldFromScreen(event.x, event.y);
        mouseEnterListener.notify(event, worldPos);
    };

    Maps.mouseLeave = function(event) {
        var worldPos = gameView.worldFromScreen(event.x, event.y);
        mouseLeaveListener.notify(event, worldPos);
    };

    /**
     * This is a rather odd function. In this case, it loads all mobiles from all maps on initial startup and
     * creates several data-structures for holding them.
     */
    Maps.load = function() {
        var legacyIdMapping = readJson('legacy_maps.js');

        /*
         Automatically load available zones. This should probably be refactored into some sort of "campaign" / "module"
         structure, which means maps only get loaded in module packages.
         */
        for (var legacyId in legacyIdMapping) {
            var mapId = legacyIdMapping[legacyId];

            var map = new Map(mapId);
            this.maps.push(map);
            this.mapsById[mapId] = map;
        }
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
            var mapObj = readJson('maps/' + map.id + '/map.js');
            if (!mapObj) {
                throw "Map " + map.id + " in savegame no longer has corresponding map file.";
            }
            mapObj.__proto__ = Map.prototype;
            map.__proto__ = mapObj;
            
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

    function updateStandpoints() {
        var isDay = GameTime.isDaytime();

        // Iterate over all mobiles that may need a standpoint change
        Maps.maps.forEach(function (map) {
            var i;
            for (i = 0; i < map.mobiles.length; ++i) {
                var mobile = map.mobiles[i];
                if (!mobile.standpointDay || !mobile.standpointNight || Party.isMember(mobile))
                    continue;

                var standpoint = mobile.standpointDay;
                if (!isDay)
                    standpoint = mobile.standpointNight;

                var newMapId = standpoint.map;
                var newPosition = standpoint.position;

                if (standpoint.jumpPoint) {
                    var jumpPoint = jumppoints[standpoint.jumpPoint];
                    if (!jumpPoint) {
                        print("Mobile " + mobile.id + " has standpoint with unknown jump point: "
                                + standpoint.jumpPoint);
                        continue;
                    }
                    newMapId = jumpPoint.map;
                    newPosition = jumpPoint.position;
                }

                if (mobile.map.id != newMapId) {
                    print("Moving mobile " + mobile.getName() + " to " + newPosition + " on map " + newMapId);

                    var newMap = Maps.mapsById[newMapId];
                    if (!newMap) {
                        print("Can't move mobile " + mobile.id + " to " + newMapId
                                + " since the map is unknown.");
                        continue;
                    }

                    mobile.map.removeMobile(mobile);
                    newMap.addMobile(mobile);
                    mobile.position = newPosition;
                    --i;
                }
            }
        });
    }

    function updateDayNight(oldHour) {
        if (Maps.currentMap) {
            Maps.currentMap.updateDayNight(oldHour);
        }

        // Move mobiles around if there was a day->night or night->day transition
        var wasNight = (oldHour < 6 || oldHour >= 18);
        var isNight = GameTime.isNighttime();

        if (wasNight != isNight) {
            updateStandpoints();
        }
    }

    StartupListeners.add(function() {
        SaveGames.addSavingListener(save);
        SaveGames.addLoadingListener(load);
        GameTime.addTimeChangedListener(updateLighting);
        GameTime.addHourChangedListener(updateDayNight);
    });

})();
