var Map = function(id) {
    if (!(this instanceof Map))
        throw "Constuct maps via the new keyword.";

    // Read the map once
    var mapObj = readJson('maps/' + id + '/map.js');

    this.id = id;
    this.visited = false;

    if (!mapObj) {
        print("Unable to load map " + id);
        return;
    }

    /*
     Store some additional information from the map file, which should be refreshed whenever the savegame is loaded.
     */
    this.name = mapObj.name;
    this.area = mapObj.area;
    this.legacyId = mapObj.legacyId; // This is *OPTIONAL*
    this.startPosition = mapObj.startPosition; // This is actually only used by the editor

    // Load mobiles and link them to their prototypes
    this.mobiles = readJson(mapObj.mobiles);

    for (var i = 0; i < this.mobiles.length; ++i) {
        var mobile = this.mobiles[i];
        connectToPrototype(mobile);
        mobile.map = this;
    }
};

(function() {

    /**
     * Searches through all mobiles in this map and tries to find a mobile using it's unique identifier (GUID).
     *
     * @param id The GUID to search for.
     */
    Map.prototype.findMobileById = function(id) {
        for (var i = 0; i < this.mobiles.length; ++i) {
            var mobile = this.mobiles[i];
            if (mobile.id == id)
                return mobile;

            // Search the mobile's content
            if (mobile.content) {
                for (var j = 0; j < mobile.content.length; ++j) {
                    var item = mobile.content[j];
                    if (item.id == id)
                        return item;
                }
            }
        }

        return null;
    };

    /**
     * Causes the heartbeat event
     */
    Map.prototype.heartbeat = function() {
        if (Maps.currentMap !== this)
            return;

        this.mobiles.forEach(function (mobile) {
            if (!mobile.OnHeartbeat)
                return;

            LegacyScripts.OnHeartbeat(mobile.OnHeartbeat, mobile);
        });

        var obj = this;
        gameView.addVisualTimer(1000, function() {
            obj.heartbeat();
        });
    };

    /**
     * Loads the static objects from a map object.
     * @param mapObj The map object loaded from a map's map.js
     */
    function loadStaticObjects(mapObj) {
        print("Creating " + mapObj.staticObjects.length + " static objects.");

        var i;
        var obj;

        for (i = 0; i < mapObj.staticObjects.length; ++i) {
            obj = mapObj.staticObjects[i];
            connectToPrototype(obj);
            obj.createRenderState();
        }
    }

    /**
     * Loads the lights from a map object.
     * @param mapObj The map object loaded from a map's map.js
     */
    function loadLights(mapObj) {
        print("Creating " + mapObj.lights.length + " lights.");

        // Create global lighting in form of an infinite-range directional light
        var globalLight = new Light(gameView.scene);
        globalLight.range = 10000000000; // The light should be visible anywhere on the map
        globalLight.type = 1;
        globalLight.color = [0.962745, 0.964706, 0.965882, 0];
        globalLight.direction = [-0.632409, -0.774634, 0, 0];

        var sceneNode = gameView.scene.createNode();
        sceneNode.position = [480 * 28, 0, 480 * 28];
        sceneNode.attachObject(globalLight);

        for (var i = 0; i < mapObj.lights.length; ++i) {
            var obj = mapObj.lights[i];

            sceneNode = gameView.scene.createNode();
            sceneNode.interactive = false;
            sceneNode.position = obj.position;

            var light = new Light(gameView.scene);
            light.type = obj.type;
            light.range = obj.range;
            // Enable this to see the range of lights
            // light.debugging = true;
            light.color = obj.color;
            sceneNode.attachObject(light);
        }
    }

    /**
     * Loads the particle systems from a given map object into the current view.
     *
     * @param mapObj The map object loaded from a map's map.js
     */
    function loadParticleSystems(mapObj) {
        print("Creating " + mapObj.particleSystems.length + " particle systems.");

        var i;
        var obj;

        for (i = 0; i < mapObj.particleSystems.length; ++i) {
            obj = mapObj.particleSystems[i];

            sceneNode = gameView.scene.createNode();
            sceneNode.interactive = false;
            sceneNode.position = obj.position;

            var particleSystem = gameView.particleSystems.instantiate(obj.name);
            sceneNode.attachObject(particleSystem);

            // makeParticleSystemTestModel(obj, sceneNode);
        }
    }

    Map.prototype.reload = function() {
        var start = timerReference();

        var mapObj = readJson('maps/' + this.id + '/map.js');

        gameView.scrollBoxMinX = mapObj.scrollBox[0];
        gameView.scrollBoxMinY = mapObj.scrollBox[1];
        gameView.scrollBoxMaxX = mapObj.scrollBox[2];
        gameView.scrollBoxMaxY = mapObj.scrollBox[3];

        gameView.backgroundMap.directory = mapObj.dayBackground;

        var scene = gameView.scene;

        gameView.clippingGeometry.load(mapObj.clippingGeometry, scene);

        gameView.sectorMap.load('maps/' + this.id + '/regions.dat');

        loadLights(mapObj);
        loadParticleSystems(mapObj);
        loadStaticObjects(mapObj);

        var obj;
        var i;

        print("Creating " + this.mobiles.length + " dynamic objects.");

        for (i = 0; i < this.mobiles.length; ++i) {
            obj = this.mobiles[i];
            obj.createRenderState();
        }

        gc();

        var elapsed = timerReference() - start;
        print("Loaded map in " + elapsed + " ms.");

        var map = this;
        gameView.addVisualTimer(1000, function() {
            map.heartbeat();
        });        
    };

    Map.prototype.entering = function(position) {
        // Ensure that the map is only entered when it's the active map
        if (Maps.currentMap !== this) {
            print("Trying to enter a map that is NOT the current map: " + this.id);
            return;
        }

        print("Party is entering map " + this.name + " (" + this.id + ")");

        this.visited = true; // Mark this map as visited

        this.reload(); // Creates the render-state and sets properties

        gameView.centerOnWorld(position);

        // Move party to starting location, add nodes to scene
        var map = this;
        Party.getMembers().forEach(function (critter) {
            critter.position = position;
            map.addMobile(critter);
            // TODO: Fire areaChanged event (NOTE: Not if loading a savegame)
        }, this);

        // Process heartbeat scripts (Only when CHANGING maps, not when loading from save!)
        for (var i = 0; i < this.mobiles.length; ++i) {
            var mobile = this.mobiles[i];
            if (mobile.OnFirstHeartbeat) {
                LegacyScripts.OnFirstHeartbeat(mobile.OnFirstHeartbeat, mobile);
            }
        }
    };

    Map.prototype.leaving = function(newMap, newPosition) {
        print("Party is leaving map " + this.name + " (" + this.id + ")");

        if (currentSelection)
            currentSelection.setSelected(false);

        gameView.scene.clear();
        renderStates = {}; // Clear render states

        var map = this;

        // Unlink all party members from this map
        Party.getMembers().forEach(function(member) {
            map.removeMobile(member);
        }, this);

        gc();
    };

    Map.prototype.persistState = function(mobile) {
        var result = {};

        // Store all properties except "mobiles", which is handled differently.
        for (var k in this) {
            if (!this.hasOwnProperty(k) || this[k] instanceof Function)
                continue; // Skip prototype properties and functions

            if (k == 'mobiles')
                continue; // Skip mobiles, handled separately

            result[k] = this[k];
        }

        result.mobiles = [];
        this.mobiles.forEach(function (mobile) {
            // A small hack that prevents party members from being saved twice
            // TODO: This should probably be done using a pre-/post- save hook from the Party class
            if (Party.isMember(mobile))
                return;
            result.mobiles.push(mobile.persist());
        });

        return result;
    };

    /**
     * Removes a top-level mobile from this map and performs the following steps:
     *
     * - Removes the mobile from this map's mobiles list.
     * - Removes the mobile's rendering state
     * - Deletes the mobile's map property.
     *
     * @param mobile The mobile to add.
     */
    Map.prototype.removeMobile = function(mobile) {
        assertTrue(mobile.map === this, "Trying to remove mobile which isn't on this map.");

        var index = this.mobiles.indexOf(mobile);

        assertTrue(index != -1, "Mobile is on this map, but not in the mobiles array.");

        this.mobiles.slice(index, 1);
        delete mobile['map'];

        mobile.removeRenderState();
    };

    /**
     * Adds a top-level mobile (no parent) to this map, and performs the following actions:
     * - Removes the mobile from its previous map if it has one.
     * - Adds the mobile to this maps mobile list.
     * - Sets the map property of the mobile to this map.
     * - Creates rendering-state for the mobile, if this map is the current map.
     *
     * @param mobile The mobile to add.
     */
    Map.prototype.addMobile = function(mobile) {
        if (mobile.map)
            mobile.map.removeMobile(mobile);

        this.mobiles.push(mobile);
        mobile.map = this;

        if (Maps.currentMap === this)
            mobile.createRenderState();
    };

})();
