var Map = function(id) {
    if (!(this instanceof Map))
        throw "Constuct maps via the new keyword.";

    // Read the map once
    var mapObj = readJson('maps/' + id + '/map.js');

    if (!mapObj)
        throw "Unable to load map " + id;

    this.id = id;
    this.visited = false;
    mapObj.__proto__ = this.__proto__;
    this.__proto__ = mapObj;

    // Load mobiles and link them to their prototypes
    if (mapObj.mobileObjects)
        this.mobiles = readJson(mapObj.mobileObjects);
    else
        this.mobiles = [];

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
     * @param staticObjects A list of static objets to load.
     */
    function loadStaticObjects(staticObjects) {
        print("Creating " + staticObjects.length + " static objects.");

        var i;
        var obj;

        for (i = 0; i < staticObjects.length; ++i) {
            obj = staticObjects[i];
            connectToPrototype(obj);
            obj.createRenderState();
        }
    }

    /**
     * Loads the lights from a map object.
     * @param map The map object to load lights for.
     */
    function loadLights(map) {

        // Create global lighting in form of an infinite-range directional light
        var globalLight = new Light(gameView.scene);
        globalLight.range = 10000000000; // The light should be visible anywhere on the map
        globalLight.type = 'Directional';
        globalLight.color = map.globalLighting.color.slice(0, 3); // The color is actually used
        globalLight.direction = [-0.6324094, -0.7746344, 0]; // the direction is fixed for *all* maps, regardless of lighting.
        var sceneNode = gameView.scene.createNode();

        sceneNode.position = [480 * 28, 0, 480 * 28];
        sceneNode.attachObject(globalLight);

        map.renderGlobalLight = globalLight;

        if (!map.lights)
            return;

        var lights = readJson(map.lights);

        print("Creating " + lights.length + " lights.");

        for (var i = 0; i < lights.length; ++i) {
            var obj = lights[i];

            sceneNode = gameView.scene.createNode();
            sceneNode.interactive = false;
            sceneNode.position = obj.position;

            var light = new Light(gameView.scene);
            switch (obj.type) {
                default:
                case 1:
                    light.type = 'Point';
                    break;
                case 2:
                    light.type = 'Spot';
                    continue;
                case 3:
                    light.type = 'Directional';
                    continue;
            }

            light.range = obj.range;
            light.attenuation = 4 / (obj.range * obj.range);
            // Enable this to see the range of lights
            // light.debugging = true;
            light.color = obj.color;

            sceneNode.attachObject(light);
        }
    }

    /**
     * Loads the particle systems from a given map object into the current view.
     *
     * @param particleSystems The particle systems.
     */
    function loadParticleSystems(particleSystems) {
        print("Creating " + particleSystems.length + " particle systems.");

        var i;
        var obj;
        var sceneNode;

        for (i = 0; i < particleSystems.length; ++i) {
            obj = particleSystems[i];

            if (!obj.day)
                continue;

            sceneNode = gameView.scene.createNode();
            sceneNode.interactive = false;
            sceneNode.position = obj.position;

            var particleSystem = gameView.particleSystems.instantiate(obj.name);
            sceneNode.attachObject(particleSystem);

            // makeParticleSystemTestModel(obj, sceneNode);
        }
    }

    Map.prototype.reload = function() {
        var map = this; // For closures

        var start = timerReference();

        gameView.scrollBoxMinX = this.scrollBox[0];
        gameView.scrollBoxMinY = this.scrollBox[1];
        gameView.scrollBoxMaxX = this.scrollBox[2];
        gameView.scrollBoxMaxY = this.scrollBox[3];

        var backgroundMap = new BackgroundMap(gameView.scene);
        if (GameTime.isNighttime() && this.nightBackground) {
            backgroundMap.directory = this.nightBackground;
        } else {
            backgroundMap.directory = this.dayBackground;
        }
        this.renderBackgroundMap = backgroundMap;

        var backgroundMapNode = gameView.scene.createNode();
        backgroundMapNode.attachObject(backgroundMap);

        var scene = gameView.scene;

        gameView.clippingGeometry.load(this.clippingGeometry, scene);

        gameView.sectorMap.load('maps/' + this.id + '/regions.dat');

        loadLights(this);
        if (this.particleSystems)
            loadParticleSystems(readJson(this.particleSystems));
        if (this.staticObjects)
            loadStaticObjects(readJson(this.staticObjects));

        var obj;
        var i;

        print("Creating " + this.mobiles.length + " dynamic objects.");

        for (i = 0; i < this.mobiles.length; ++i) {
            obj = this.mobiles[i];
            obj.createRenderState();
        }

        if (!this.unfogged) {
            // TODO: Load/Persist FOW state
            this.renderFog = new FogOfWar(gameView.scene);
            this.renderFog.sectorMap = gameView.sectorMap;
            var fowNode = gameView.scene.createNode();
            fowNode.attachObject(this.renderFog);
        }

        gc();

        var elapsed = timerReference() - start;
        print("Loaded map in " + elapsed + " ms.");

        gameView.addVisualTimer(1000, function() {
            map.heartbeat();
        });

        SoundController.activate(this.soundSchemes);
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

            // Fire the san_new_map events for NPCs
            if (critter.OnNewMap) {
                print("Performing new map callback for " + critter.id);
                LegacyScripts.OnNewMap(critter.OnNewMap, critter);
            }

        }, this);

        // TODO: Figure out if initial heartbeats are triggered before or after the new_area events

        // Process heartbeat scripts (Only when CHANGING maps, not when loading from save!)
        for (var i = 0; i < this.mobiles.length; ++i) {
            var mobile = this.mobiles[i];
            if (mobile.OnFirstHeartbeat) {
                LegacyScripts.OnFirstHeartbeat(mobile.OnFirstHeartbeat, mobile);
            }
        }

        UtilityBarUi.update();
    };

    Map.prototype.leaving = function(newMap, newPosition) {
        print("Party is leaving map " + this.name + " (" + this.id + ")");

        delete this['renderBackgroundMap'];
        delete this['renderGlobalLight'];

        gameView.scene.clear();
        renderStates = {}; // Clear render states

        // Unlink all party members from this map
        Party.getMembers().forEach(function(member) {
            this.removeMobile(member);
        }, this);

        gc();

        UtilityBarUi.update();
    };

    Map.prototype.persistState = function(mobile) {
        var result = {};

        // Store all properties except "mobiles", which is handled differently.
        for (var k in this) {
            if (!this.hasOwnProperty(k) || this[k] instanceof Function)
                continue; // Skip prototype properties and functions

            if (k == 'renderGlobalLight' || k == 'renderBackgroundMap')
                continue; // Render states should be skipped

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

        var removed = this.mobiles.splice(index, 1);
        assertTrue(removed.length == 1, "Didn't remove mobile from list.");
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

    /**
     * Interpolates a color value from key-frames color information.
     * @param hour The current hour, as a decimal.
     * @param keyframedColorTable The lighting map.
     */
    function interpolateColor(hour, keyframedColorTable) {
        var color;
        var factor, invFactor;
        var i;
        var prevTime, prevColor, nextTime, nextColor;

        if (hour <= keyframedColorTable[0][0]) {
            color = keyframedColorTable[0][1];
        } else if (hour >= keyframedColorTable[keyframedColorTable.length - 1][0]) {
            color = keyframedColorTable[keyframedColorTable.length - 1][1];
        } else {
            // Find the previous/next index
            for (i = 0; i < keyframedColorTable.length - 1; ++i) {
                if (keyframedColorTable[i + 1][0] >= hour) {
                    prevTime = keyframedColorTable[i][0];
                    prevColor = keyframedColorTable[i][1];
                    nextTime = keyframedColorTable[i + 1][0];
                    nextColor = keyframedColorTable[i + 1][1];
                    break;
                }
            }

            factor = (hour - prevTime) / (nextTime - prevTime);
            invFactor = 1 - factor;

            color = [invFactor * prevColor[0] + factor * nextColor[0],
                invFactor * prevColor[1] + factor * nextColor[1],
                invFactor * prevColor[2] + factor * nextColor[2]];
        }

        return color;
    }

    /**
     * This map fading is a huge mess and definetly needs to be refactored.
     * Some pointers:
     * Either modify the background map class in a way that allows direct cross-fading between
     * two maps, or move this into a separate javascript class (MapCrossFader?).
     * Also something that needs to be considered: What happens, if during a cross-fade,
     * time is passed, and another cross-fade needs to happen?
     * Also: This could probably/possibly be moved into updateLighting, and the transition be stretched
     * out across 15-30 ingame minutes to make it seem more natural.
     */

    var fadingOutMap;
    var fadingOutMapNode;
    var fadingStart;
    var fading3dStartColor;
    var fading3dEndColor;
    var fadingDuration = 1000;

    function getForegroundFade() {
        if (!fadingStart)
            return 1;

        var now = timerReference();
        return (now - fadingStart) / fadingDuration;
    }

    function mapFade() {
        var now = timerReference();

        if (now >= fadingStart + fadingDuration) {
            fadingStart = undefined;
            gameView.scene.removeNode(fadingOutMapNode);
            fadingOutMapNode = null;
            fadingOutMap = null;
            Maps.currentMap.updateLighting();
            return;
        }

        var factor = (now - fadingStart) / fadingDuration;

        var color = Maps.currentMap.renderBackgroundMap.color;

        // Fade between day/night background map
        Maps.currentMap.renderBackgroundMap.color = [ color[0], color[1], color[2], factor ];
        color = fadingOutMap.color;
        fadingOutMap.color = [ color[0], color[1], color[2], 1 - factor ];

        // Interpolate between 3d colors
        var diff = [ fading3dEndColor[0] - fading3dStartColor[0],
            fading3dEndColor[1] - fading3dStartColor[1],
            fading3dEndColor[2] - fading3dStartColor[2] ];

        Maps.currentMap.renderGlobalLight.color = [
            fading3dStartColor[0] + factor * diff[0],
            fading3dStartColor[1] + factor * diff[1],
            fading3dStartColor[2] + factor * diff[2]
        ];

        gameView.addVisualTimer(50, mapFade);
    }

    /**
     * Updates lighting for this map based on the current game time.
     */
    Map.prototype.updateLighting = function() {
        if (fadingStart)
            return; // No lighting updates while cross-fading is in effect

        // This flag is probably *NOT* used. Instead, I guess the existence of a daylight.mes rule should suffice.
        // TODO: How do we know how to alias this? I.e. the tower top in hommlet has no entry, but uses homletts entry
        // instead. Same thing for the moathouse top-level
        if (!this.globalLighting.day2dKeyframes) {
            this.renderBackgroundMap.color = [1, 1, 1];
            return;
        }

        var keyframes2d, keyframes3d;

        if (GameTime.isDaytime()) {
            keyframes2d = this.globalLighting.day2dKeyframes;
            keyframes3d = this.globalLighting.day3dKeyframes;
        } else {
            keyframes2d = this.globalLighting.night2dKeyframes;
            keyframes3d = this.globalLighting.night3dKeyframes;
        }

        if (!keyframes2d || !keyframes3d) {
            print("Map has underspecified lighting keyframes: " + this.id);
        }

        var hour = GameTime.getHourOfDay() + GameTime.getMinuteOfHour() / 60;

        var color = interpolateColor(hour, keyframes2d).slice(0);
        color[3] = getForegroundFade();
        this.renderBackgroundMap.color = color;

        this.renderGlobalLight.color = interpolateColor(hour, keyframes3d);
    };

    /**
     * Update the day/night cycle.
     * @param oldHour The hour we're coming from.
     */
    Map.prototype.updateDayNight = function(oldHour) {
        var newHour = GameTime.getHourOfDay();
        print("New Hour: " + newHour + " Old Hour: " + oldHour);

        var oldHourWasNight = oldHour < 6 || oldHour >= 18;
        var newHourIsNight = newHour < 6 || newHour >= 18;

        if (oldHourWasNight && !newHourIsNight) {
            print("TRANSITION NIGHT->DAY");
            // No background fade is necessary if there's no night background
            if (this.nightBackground) {
                fadingOutMap = new BackgroundMap(gameView.scene);
                fadingOutMap.directory = this.nightBackground;
                if (this.globalLighting.night2dKeyframes)
                    fadingOutMap.color = interpolateColor(newHour, this.globalLighting.night2dKeyframes);
                fadingOutMapNode = gameView.scene.createNode();
                fadingOutMapNode.attachObject(fadingOutMap);
                fadingStart = timerReference();

                if (this.globalLighting.night3dKeyframes)
                    fading3dStartColor = interpolateColor(newHour, this.globalLighting.night3dKeyframes);
                else
                    fading3dStartColor = this.globalLighting.color;
                if (this.globalLighting.day3dKeyframes)
                    fading3dEndColor = interpolateColor(newHour, this.globalLighting.day3dKeyframes);
                else
                    fading3dEndColor = this.globalLighting.color;

                print("Switching background map to: " + this.dayBackground);
                this.renderBackgroundMap.directory = this.dayBackground;
                this.updateLighting();
                gameView.addVisualTimer(50, mapFade);
            }
        } else if (!oldHourWasNight && newHourIsNight) {
            print("TRANSITION DAY->NIGHT");
            if (this.nightBackground) {
                print("Switching background map to: " + this.nightBackground);

                fadingOutMap = new BackgroundMap(gameView.scene);
                fadingOutMap.directory = this.dayBackground;
                if (this.globalLighting.day2dKeyframes)
                    fadingOutMap.color = interpolateColor(newHour, this.globalLighting.day2dKeyframes);
                fadingOutMapNode = gameView.scene.createNode();
                fadingOutMapNode.attachObject(fadingOutMap);
                fadingStart = timerReference();

                if (this.globalLighting.day3dKeyframes)
                    fading3dStartColor = interpolateColor(newHour, this.globalLighting.day3dKeyframes);
                else
                    fading3dStartColor = this.globalLighting.color;
                if (this.globalLighting.night3dKeyframes)
                    fading3dEndColor = interpolateColor(newHour, this.globalLighting.night3dKeyframes);
                else
                    fading3dEndColor = this.globalLighting.color;

                this.renderBackgroundMap.directory = this.nightBackground;
                this.updateLighting();
                gameView.addVisualTimer(50, mapFade);
            }
        }
    };

    StartupListeners.add(function() {

        var fogCheck = function() {
            if (Maps.currentMap && Maps.currentMap.renderFog) {
                Maps.currentMap.renderFog.reveal(Party.getLeader().position, 100);
            }

            gameView.addVisualTimer(500, fogCheck);
        };

        gameView.addVisualTimer(500, fogCheck);

    });

})();
