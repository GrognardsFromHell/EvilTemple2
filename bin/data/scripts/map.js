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
     Store some additional information from the map file, which should be refreshed everytime the save game is loaded
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

Map.prototype.entering = function(position) {
    // Ensure that the map is only entered when it's the active map
    if (Maps.currentMap !== this) {
        print("Trying to enter a map that is NOT the current map: " + this.id);
        return;
    }

    print("Party is entering map " + this.name + " (" + this.id + ")");

    this.visited = true; // Mark this map as visited

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

    // gameView.sectorMap.createDebugView();

    print("Creating " + mapObj.staticObjects.length + " static objects.");

    var obj;

    for (var i = 0; i < mapObj.staticObjects.length; ++i) {
        obj = mapObj.staticObjects[i];
        connectToPrototype(obj);
        createMapObject(scene, obj);
    }

    print("Creating " + this.mobiles.length + " dynamic objects.");

    for (var i = 0; i < this.mobiles.length; ++i) {
        obj = this.mobiles[i];
        createMapObject(scene, obj);
    }

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
        obj = mapObj.lights[i];

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

    print("Creating " + mapObj.particleSystems.length + " particle systems.");

    for (var i = 0; i < mapObj.particleSystems.length; ++i) {
        obj = mapObj.particleSystems[i];

        var sceneNode = gameView.scene.createNode();
        sceneNode.interactive = false;
        sceneNode.position = obj.position;

        var particleSystem = gameView.particleSystems.instantiate(obj.name);
        sceneNode.attachObject(particleSystem);

        // makeParticleSystemTestModel(obj, sceneNode);
    }

    // Move party to starting location, add nodes to scene
    var map = this;
    Party.getMembers().forEach(function (critter) {
        critter.map = this;
        critter.position = position;
        map.mobiles.push(critter);
        createMapObject(scene, critter);

        // Fire areaChanged event (NOTE: Not if loading a savegame)
    }, this);

    gameView.centerOnWorld(position);

    gc();

    var elapsed = timerReference() - start;
    print("Loaded map in " + elapsed + " ms.");

    // Process heartbeat scripts (Only when CHANGING maps, not when loading from save!)
    for (i = 0; i < this.mobiles.length; ++i) {
        var mobile = this.mobiles[i];
        if (mobile.OnFirstHeartbeat) {
            LegacyScripts.OnFirstHeartbeat(mobile.OnFirstHeartbeat, mobile);
        }
    }
    
    gameView.addVisualTimer(1000, function() {
        map.heartbeat();
    });    
};

Map.prototype.leaving = function(newMap, newPosition) {
    print("Party is leaving map " + this.name + " (" + this.id + ")");

    // Unlink all party members from this map
    Party.getMembers().forEach(function(member) {
        var idx = this.mobiles.indexOf(member);
        if (idx != -1)
            this.mobiles.splice(idx, 1);
        member.map = undefined;
    }, this);

    if (currentSelection != null) {
        currentSelection.setSelected(false);
    }
    gameView.scene.clear();
    renderStates = {}; // Clear render states
    gc();
};

Map.prototype.addMobile = function(mobile) {
    this.removeMobile(mobile); // Make sure there are no duplicates
    this.mobiles.push(mobile);
};

Map.prototype.removeMobile = function(mobile) {
    var idx = this.mobiles.indexOf(mobile);

    if (idx != -1)
        this.mobiles.splice(idx, 1);
};
