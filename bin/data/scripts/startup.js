
var models; // Will be aliased to gameView.models

//var startupMap = 'maps/Map12-temple-dungeon-level-1/map.js';
// var startupMap = 'maps/Map-2-Hommlet-Exterior/map.js';
// var startupMap = 'maps/Map-7-Moathouse_Interior/map.js';
var startupMap = 'maps/Map-9-Nulb-Exterior/map.js';

var maps = [
    { name: 'Temple Level 1', dir: 'Map12-temple-dungeon-level-1' },
    { name: "Temple Level 1 (Room)", dir: 'Map12-temple-dungeon-level-1-room-131' },
    { name: "Temple Level 2", dir: "Map13-dungeon-level-02" },
    { name: "Temple Level 3 (Lower)", dir: "Map14-dungeon-level-03_lower" },
    { name: "Temple Level 3 (Upper)", dir: "Map14-dungeon-level-03_upper" },
    { name: "Temple Level 4", dir: "Map15-dungeon-level-04" },
    { name: 'Hommlet', dir: 'Map-2-Hommlet-Exterior' },
    { name: 'Moathouse Interior', dir: 'Map-7-Moathouse_Interior' },
    { name: 'Nulb', dir: 'Map-9-Nulb-Exterior' },
    { name: 'Air Node', dir: 'Map16-air-node' },
    { name: 'Fire Node', dir: 'Map18-fire-node' },
    { name: 'Water Node', dir: 'Map19-water-node' },
    { name: 'Colosseum', dir: 'Map-49-Colosseum' },
    { name: 'Emridy Meadows', dir: 'Map-Area-5-Emridy-Meadows' },
    { name: 'Imeryds Run', dir: 'Map-Area-6-Imeryds-Run' },
    { name: 'Ogre Cave', dir: 'Map-Area-9-Ogre-Cave-Exterior' },
    { name: 'Decklo Grove', dir: 'Map-Area-10-Decklo-Grove' },
    { name: 'Tutorial Map', dir: 'Tutorial Map 1' },
    { name: 'Vignette Lawful Good', dir: 'Vignette-Lawful-Good' },
    { name: 'Vignette Good', dir: 'Vignette-Good' },
    { name: 'Vignette Chaotic-Good', dir: 'Vignette-Chaotic-Good' },
    { name: 'Vignette Lawful', dir: 'Vignette-Lawful' },
    { name: 'Vignette Neutral', dir: 'Vignette-Neutral' },
    { name: 'Vignette Chaotic', dir: 'Vignette-Chaotic' },
    { name: 'Vignette Lawful Evil', dir: 'Vignette-Lawful-Evil' },
    { name: 'Vignette Evil', dir: 'Vignette-Evil' },
    { name: 'Vignette Chaotic Evil', dir: 'Vignette-Chaotic-Evil' },
    { name: 'Random Riverside Road', dir: 'Random-Riverside-Road' }
];

var jumppoints = {};

function objectToString(value) {
    var result = '{';
    for (var sk in value) {
        if (result != '{')
            result += ', ';
        result += sk + ': ' + value[sk];
    }
    result += '}';
    return result;
}

var currentSelection = null;

// Base object for all prototypes
var BaseObject = {
    scale: 100,
    rotation: 0,
    interactive: true,
    drawBehindWalls: false,

    registerHandlers: function(sceneNode, modelInstance) {
        if (this.selectionCircle !== undefined) {
            this.selectionCircle.mousePressed.connect(this, this.clicked);
        }

        modelInstance.mousePressed.connect(this, this.clicked);
    },

    setSelected: function(selected) {
        this.renderState.selectionCircle.selected = selected;
    },

    clicked: function() {
        if (currentSelection != null)
            currentSelection.setSelected(false);

        currentSelection = this;

        this.setSelected(true);

        showMobileInfo(this, this.renderState.modelInstance);
    },

    getReactionColor: function() {
        return [1, 1, 0]; // Neutral color
    }
};

var Portal = {
    __proto__: BaseObject,
    interactive: true,
    open: false, // All doors are shut by default
    onClicked: function(sceneNode, renderable) {
        if (this.open) {
            print("Closing door.");
            renderable.playAnimation('close', false);
            renderable.idleAnimation = 'item_idle';
        } else {
            print("Opening door.");
            if (!renderable.playAnimation('open', false)) {
                print("Unable to play open animation for door.");
            }
            renderable.idleAnimation = 'open_idle';
        }
        this.open = !this.open;
    },
    registerHandlers: function(sceneNode, renderable) {
        var obj = this;
        renderable.mousePressed.connect(function() {
            obj.onClicked(sceneNode, renderable);
        });
    }
};

var MapChanger = {
    __proto__: BaseObject,
    interactive: true,
    onClicked: function() {
        var jumpPoint = jumppoints[this.teleportTarget];
        loadMap('maps/' + jumpPoint.map + '/map.js');
        gameView.centerOnWorld(jumpPoint.x, jumpPoint.z);
    },
    registerHandlers: function(sceneNode, modelInstance) {
        var obj = this;
        modelInstance.mousePressed.connect(function() {
            obj.onClicked();
        });
    }
};

var Critter = {
    __proto__: BaseObject,
    concealed: false,
    getReactionColor: function() {
        return [0.33, 1, 0]; // Friendly
    },
    drawBehindWalls: true
};

var NonPlayerCharacter = {
    __proto__: Critter
};

var prototypes;
var sounds;

function startup() {
    models = gameView.models;

    print("Showing main menu.");

    var mainMenu = gameView.showView("interface/MainMenu.qml");
    mainMenu.newGameClicked.connect(function() {
        mainMenu.deleteLater();
        showDebugBar();
    });

    print("Loading prototypes...");
    prototypes = eval('(' + readFile('prototypes.js') + ')');
    prototypes['StaticGeometry'] = {
        interactive: false
    };
    print("Loading jump points...");
    jumppoints = eval('(' + readFile('jumppoints.js') + ')');
    loadEquipment();
    loadPortraits();
    loadInventoryIcons();
    sounds = eval('(' + readFile('sound/sounds.js') + ')');

    // Assign the prototype of each loaded prototype
    for (var i in prototypes) {
        var type = prototypes[i].type;

        if (type == 'MapChanger')
            prototypes[i].__proto__ = MapChanger;
        else if (type == 'Portal')
            prototypes[i].__proto__ = Portal;
        else if (type == 'NonPlayerCharacter')
            prototypes[i].__proto__ = NonPlayerCharacter;
        else
            prototypes[i].__proto__ = BaseObject;
    }

    // Accept clicks from the game view
    setupWorldClickHandler();
}

var worldClickCallback = null;

function selectWorldTarget(callback) {
    worldClickCallback = callback;
}

function setupWorldClickHandler() {
    var firstClick = undefined;

    gameView.worldClicked.connect(function(worldPosition) {
        if (worldClickCallback != null) {
            var callback = worldClickCallback;
            worldClickCallback = null;
            callback(worldPosition);
            return;
        }

        var color;
        var material = gameView.sectorMap.regionTag("groundMaterial", worldPosition);

        if (currentSelection != null) {
            walkTo(currentSelection, worldPosition);
            return;
        }

        if (firstClick === undefined) {
            var text = "1st @ " + Math.floor(worldPosition[0]) + "," + Math.floor(worldPosition[2]) + " (" + material + ")";
            gameView.scene.addTextOverlay(worldPosition, text, [0.9, 0.9, 0.9, 0.9]);
            firstClick = worldPosition;
        } else {
            var inLos = gameView.sectorMap.hasLineOfSight(firstClick, worldPosition);
            if (inLos) {
                color = [0.2, 0.9, 0.2, 0.9];
            } else {
                color = [0.9, 0.2, 0.2, 0.9];
            }

            var text = "2nd @ " + Math.floor(worldPosition[0]) + "," + Math.floor(worldPosition[2]) + " (" + material + ")";
            gameView.scene.addTextOverlay(worldPosition, text, color);

            var path = gameView.sectorMap.findPath(firstClick, worldPosition);

            print("Found path of length: " + path.length);

            var sceneNode = gameView.scene.createNode();

            for (var i = 0; i < path.length; ++i) {
                if (i + 1 < path.length) {
                    var line = new LineRenderable(gameView.scene);
                    line.addLine(path[i], path[i+1]);
                    sceneNode.attachObject(line);
                }

                gameView.scene.addTextOverlay(path[i], 'X', [1, 0, 0], 1);
            }

            gameView.addVisualTimer(5000, function() {
                print("Removing scene node.");
                gameView.scene.removeNode(sceneNode);
            });

            firstClick = undefined;
        }
    });
}

var animEventGameFacade = {
    particles: function(partSysId, proxyObject) {
        var modelInstance = proxyObject.modelInstance;
        var sceneNode = proxyObject.sceneNode;
        var particleSystem = gameView.particleSystems.instantiate(partSysId);
        particleSystem.modelInstance = modelInstance;
        sceneNode.attachObject(particleSystem);
    },
    sound_local_obj: function(soundId, sceneNode) {
        var filename = sounds[soundId];
        if (filename === undefined) {
            print ("Unknown sound id: " + soundId);
            return;
        }

        print("Playing sound " + filename)
        gameView.audioEngine.playSoundOnce(filename, SoundCategory_Effect);
    },
    shake: function(a, b) {
        print("Shake: " + a + ", " + b);
    }
};

var footstepSounds = {
    'dirt': ['sound/footstep_dirt1.wav', 'sound/footstep_dirt2.wav', 'sound/footstep_dirt3.wav', 'sound/footstep_dirt4.wav'],
    'sand': ['sound/footstep_sand1.wav', 'sound/footstep_sand2.wav', 'sound/footstep_sand3.wav', 'sound/footstep_sand4.wav'],
    'ice': ['sound/footstep_snow1.wav', 'sound/footstep_snow2.wav', 'sound/footstep_snow3.wav', 'sound/footstep_snow4.wav'],
    'stone': ['sound/footstep_stone1.wav', 'sound/footstep_stone2.wav', 'sound/footstep_stone3.wav', 'sound/footstep_stone4.wav'],
    'water': ['sound/footstep_water1.wav', 'sound/footstep_water2.wav', 'sound/footstep_water3.wav', 'sound/footstep_water4.wav'],
    'wood': ['sound/footstep_wood1.wav', 'sound/footstep_wood2.wav', 'sound/footstep_wood3.wav', 'sound/footstep_wood4.wav']
};

var footstepCounter = 0;

var animEventAnimObjFacade = {
    footstep: function() {
        // TODO: Should we use Bip01 Footsteps bone here?
        var material = gameView.sectorMap.regionTag("groundMaterial", this.sceneNode.position);

        if (material === undefined)
            return;

        var sounds = footstepSounds[material];

        if (sounds === undefined) {
            print("Unknown material-type: " + material);
            return;
        }

        var sound = sounds[footstepCounter++ % sounds.length];

        gameView.audioEngine.playSoundOnce(sound, SoundCategory_Effect);
    }
};

function handleAnimationEvent(sceneNode, modelInstance, obj, type, content)
{
    var game = animEventGameFacade;

    var anim_obj = {
        sceneNode: sceneNode,
        modelInstance: modelInstance,
        obj: obj
    };
    anim_obj.__proto__ = animEventAnimObjFacade;

    /*
        Python one-liners are in general valid javascript, so we directly evaluate them here.
        Eval has access to all local variables in this scope, so we can define game + anim_obj,
        which are the most often used by the animation events.
    */
    eval(content);
}

function createMapObject(scene, obj)
{
    // This is clumsy and needs preprocessing
    if (obj.flags !== undefined) {
        for (var i = 0; i < obj.flags.length; ++i) {
            if (obj.flags[i] == 'ClickThrough')
                obj.interactive = false;
        }
    }

    var sceneNode = gameView.scene.createNode();
    sceneNode.interactive = obj.interactive;
    sceneNode.position = obj.position;
    sceneNode.rotation = rotationFromDegrees(obj.rotation);
    var scale = obj.scale / 100.0;
    sceneNode.scale = [scale, scale, scale];

    var modelObj = models.load(obj.model);

    var modelInstance = new ModelInstance(gameView.scene);
    modelInstance.model = modelObj;
    modelInstance.drawBehindWalls = obj.drawBehindWalls;
    updateEquipment(obj, modelInstance);
    modelInstance.animationEvent.connect(function(type, content) {
        handleAnimationEvent(sceneNode, modelInstance, obj, type, content);
    });

    // Store render state with the object
    obj.renderState = {
        modelInstance: modelInstance,
        sceneNode: sceneNode
    };

    if (obj.interactive) {
        var selectionCircle = new SelectionCircle(scene, gameView.materials);
        obj.renderState.selectionCircle = selectionCircle;

        if (obj.radius !== undefined)
            selectionCircle.radius = obj.radius;

        selectionCircle.color = obj.getReactionColor();

        sceneNode.attachObject(selectionCircle);
        obj.selectionCircle = selectionCircle;

        obj.registerHandlers(sceneNode, modelInstance);
    }
    sceneNode.attachObject(modelInstance);
}

function makeParticleSystemTestModel(particleSystem, sceneNode) {
    var testModel = models.load('meshes/scenery/misc/mirror.model');
    var modelInstance = new ModelInstance(gameView.scene);
    modelInstance.model = testModel;
    sceneNode.interactive = true;
    modelInstance.mousePressed.connect(function() {
        print(particleSystem.name);
    });
    sceneNode.attachObject(modelInstance);
}

function loadMap(filename) {
    var start = timerReference();

    currentSelection = null;
    gameView.scene.clear();
    gc();

    var mapObj = readJson(filename);

    gameView.scrollBoxMinX = mapObj.scrollBox[0];
    gameView.scrollBoxMinY = mapObj.scrollBox[1];
    gameView.scrollBoxMaxX = mapObj.scrollBox[2];
    gameView.scrollBoxMaxY = mapObj.scrollBox[3];

    gameView.backgroundMap.directory = mapObj.dayBackground;

    var scene = gameView.scene;

    gameView.clippingGeometry.load(mapObj.clippingGeometry, scene);

    gameView.sectorMap.load(filename.replace('map.js', '') + 'regions.dat');

    // gameView.sectorMap.createDebugView();

    print("Creating " + mapObj.staticObjects.length + " static objects.");

    var obj;

    for (var i = 0; i < mapObj.staticObjects.length; ++i) {
        obj = mapObj.staticObjects[i];
        connectToPrototype(obj);
        createMapObject(scene, obj);
    }

    print("Creating " + mapObj.dynamicObjects.length + " dynamic objects.");

    for (var i = 0; i < mapObj.dynamicObjects.length; ++i) {
        obj = mapObj.dynamicObjects[i];
        connectToPrototype(obj);
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
    sceneNode.position = [480 * 28, 0, 480*28];
    sceneNode.attachObject(globalLight);

    for (var i = 0; i < mapObj.lights.length; ++i) {
        obj = mapObj.lights[i];

        var sceneNode = gameView.scene.createNode();
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

        /*
            Debugging code
        */
        // makeParticleSystemTestModel(obj, sceneNode);
    }

    /*print("Creating debug objects for waypoints.");
    for (var waypointId in mapObj.waypoints) {
        var waypoint = mapObj.waypoints[waypointId];

        var sceneNode = gameView.scene.createNode();
        sceneNode.interactive = false;
        sceneNode.position = [waypoint.x, 0, waypoint.y];

        var testModel = models.load('meshes/items/Ale_stien.model');

        var modelInstance = new ModelInstance(gameView.scene);
        modelInstance.model = testModel;
        sceneNode.attachObject(modelInstance);

        var origin = [];

        // Connect with targets
        for (var i = 0; i < waypoint.goals.length; ++i) {
            var goal = mapObj.waypoints[waypoint.goals[i]];
            var diffX = goal.x - waypoint.x;
            var diffY = goal.y - waypoint.y;

            var line = new LineRenderable(gameView.scene);
            line.addLine(origin, [diffX, 0, diffY]);
            sceneNode.attachObject(line);
        }
    }*/

    gameView.centerOnWorld(mapObj.startPosition[0], mapObj.startPosition[2]);

    gc();

    var elapsed = timerReference() - start;
    print("Loaded map in " + elapsed + " ms.");
}

function connectToPrototype(obj) {
    if (obj.prototype !== undefined) {
        obj.__proto__ = prototypes[obj.prototype];
    } else {
        obj.__proto__ = BaseObject;
    }

    if (obj.content !== undefined) {
        for (var i = 0; i < obj.content.length; ++i) {
            connectToPrototype(obj.content[i]);
        }
    }
}
