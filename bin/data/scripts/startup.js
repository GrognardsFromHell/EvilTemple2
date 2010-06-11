
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

// Base object for all prototypes
var BaseObject = {
    scale: 100,
    rotation: 0,
    interactive: true,
    registerHandlers: function(sceneNode, modelInstance) {
        var obj = this;
        modelInstance.setClickHandler(function() {
            var mobileInfoDialog = gameView.addGuiItem("interface/MobileInfo.qml");
            var items = [];

            for (var k in obj) {
                // Content is handled by inventory
                if (k == 'content')
                    continue;

                var value = obj[k];
                // Skip functions
                if (value instanceof Function)
                    continue;
                if (value instanceof Array) {
                    var newValue = '[';
                    for (var i = 0; i < value.length; ++i) {
                        var subvalue = value[i];

                        if (typeof(subvalue) == 'object')
                            subvalue = objectToString(subvalue);

                        if (i != 0)
                            newValue += ', ' + subvalue;
                        else
                            newValue += subvalue;
                    }
                    value = newValue + ']';
                } else if (typeof(value) == 'object') {
                    value = objectToString(value);
                }
                if (k == 'descriptionId') {
                    value = translations.get('mes/description/' + value) + ' (' + value + ')';
                } else if (k == 'unknownDescriptionId') {
                    value = translations.get('mes/description/' + value) + ' (' + value + ')';
                }
                items.push([k, value]);
            }

            mobileInfoDialog.setPortrait(getPortrait(obj.portrait, Portrait_Medium));
            mobileInfoDialog.title = 'Property View';
            mobileInfoDialog.setItems(items);
            mobileInfoDialog.closeClicked.connect(function() {
                mobileInfoDialog.deleteLater();
            });
            
            mobileInfoDialog.openAnimations.connect(function() {
                openAnimations(modelInstance);
            });
            
            mobileInfoDialog.hasInventory = (obj.content !== undefined && obj.content.length > 0);            
            mobileInfoDialog.openInventory.connect(function() {				           
                if (obj.content !== undefined && obj.content.length > 0) {
                    var inventoryDialog = gameView.addGuiItem("interface/Inventory.qml");	

                    var gold = 0;
                    var silver = 0;
                    var copper = 0;
                    var platinum = 0;
                    
                    var objects = [];
                    for (var i = 0; i < obj.content.length; ++i) {
                        var item = obj.content[i];
                        if (item.inventoryId === undefined || item.descriptionId === undefined) {
                            print("Object lacks inventory id or description id.");
                            continue;
                        }

                        var quantity = 1;
                        if (item.moneyQuantity !== undefined)
                            quantity = item.moneyQuantity;
                        if (item.ammoQuantity !== undefined)
                            quantity = item.ammoQuantity;

                        if (item.type == 'Money') {
                            // This should be fixed
                            switch (item.prototype) {
                            case 7000:
                                    copper += quantity;
                                break;
                            case 7001:
                                    silver += quantity;
                                break;
                            case 7002:
                                    gold += quantity;
                                break;
                            case 7003:
                                    platinum += quantity;
                                break;
                            default:
                                    print("Unknown money prototype: " + item.prototype);
                                break;
                            }						
                            continue; // Don't show money in the inventory
                        }

                        var magical = false;
                        if (item.itemFlags !== undefined) {
                            for (var j= 0; j < item.itemFlags.length; ++j) {
                                if (item.itemFlags[j] == 'IsMagical') {
                                    magical = true;
                                    break;
                                }
                            }
                        }

                        objects.push({
                            iconPath: getInventoryIconPath(item.inventoryId),
                            description: translations.get('mes/description/' + item.descriptionId),
                            location: item.itemInventoryLocation,
                            quantity: quantity,
                            magical: magical
                        });
                    }

                    inventoryDialog.setInventory({
                        gold: gold,
                        silver: silver,
                        platinum: platinum,
                        copper: copper,
                        items: objects
                    });
                    inventoryDialog.closeClicked.connect(function() {
                        inventoryDialog.deleteLater();
                    });
                } else {
                    print("Object has no contents. Skipping.");
                }
            });		
        });
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
        renderable.setClickHandler(function() {
            obj.onClicked(sceneNode, renderable);
        });
    }
};

var MapChanger = {
    __proto__: BaseObject,
    interactive: true,
    onClicked: function() {
        var jumpPoint = jumppoints[this.teleportTarget];
        gameView.scene.clear();
        loadMap('maps/' + jumpPoint.map + '/map.js');
        gameView.centerOnWorld(jumpPoint.x, jumpPoint.z);
    },
    registerHandlers: function(sceneNode, modelInstance) {
        var obj = this;
        modelInstance.setClickHandler(function() {
            obj.onClicked();
        });
    }
};

var prototypes;
var sounds;

function startup() {
    print("Showing main menu.");

    var mainMenu = gameView.showView("interface/MainMenu.qml");
    mainMenu.newGameClicked.connect(function() {
        mainMenu.deleteLater();
        var loadMapUi = gameView.addGuiItem('interface/LoadMap.qml');
        loadMapUi.setMapList(maps);
        loadMapUi.mapSelected.connect(function(dir) {
            gameView.scene.clear();
            loadMap('maps/' + dir + '/map.js');
        });
        loadMapUi.closeClicked.connect(function() {
            loadMapUi.deleteLater();
        });
    });

    print("Loading prototypes...");
    prototypes = eval('(' + readFile('prototypes.js') + ')');
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
        else
            prototypes[i].__proto__ = BaseObject;
    }
}

function rotationFromDegrees(degrees) {
    var radians = degrees * 0.0174532925199432;
    var cosRot = Math.cos(radians / 2);
    var sinRot = Math.sin(radians / 2);
    return new Quaternion(0, sinRot, 0, cosRot);
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

    /*
        Python one-liners are in general valid javascript, so we directly evaluate them here.
        Eval has access to all local variables in this scope, so we can define game + anim_obj,
        which are the most often used by the animation events.
    */
    eval(content);
}

function createMapObject(scene, obj)
{
    var sceneNode = new SceneNode();
    sceneNode.interactive = obj.interactive;
    sceneNode.position = new Vector4(obj.position[0], obj.position[1], obj.position[2], 1);
    sceneNode.rotation = rotationFromDegrees(obj.rotation);
    var scale = obj.scale / 100.0;
    sceneNode.scale = new Vector4(scale, scale, scale, 0);
    scene.addNode(sceneNode);

    var modelObj = gameView.loadModel(obj.model);

    var modelInstance = new ModelInstance();
    modelInstance.model = modelObj;
    updateEquipment(obj, modelInstance);
    modelInstance.setAnimationEventHandler(function(type, content) {
        handleAnimationEvent(sceneNode, modelInstance, obj, type, content);
    });
    if (obj.interactive)
        obj.registerHandlers(sceneNode, modelInstance);
    sceneNode.attachObject(modelInstance);
}

function loadMap(filename) {
    print("Loading map " + filename);

    var mapFile = readFile(filename);
    if (mapFile === undefined) {
        print("Unable to open map file.");
        return;
    }

    var mapObj = eval('(' + mapFile + ')');

    print(mapObj.name);

    gameView.scrollBoxMinX = mapObj.scrollBox[0];
    gameView.scrollBoxMinY = mapObj.scrollBox[1];
    gameView.scrollBoxMaxX = mapObj.scrollBox[2];
    gameView.scrollBoxMaxY = mapObj.scrollBox[3];

    gameView.backgroundMap.directory = mapObj.dayBackground;

    var scene = gameView.scene;

    gameView.clippingGeometry.load(mapObj.clippingGeometry, scene);

    print("Creating " + mapObj.staticObjects.length + " static objects.");

    for (var i = 0; i < mapObj.staticObjects.length; ++i) {
        var obj = mapObj.staticObjects[i];
        connectToPrototype(obj);
        createMapObject(scene, obj);
    }

    print("Creating " + mapObj.dynamicObjects.length + " dynamic objects.");

    for (var i = 0; i < mapObj.dynamicObjects.length; ++i) {
        var obj = mapObj.dynamicObjects[i];
        connectToPrototype(obj);
        createMapObject(scene, obj);
    }

    print("Creating " + mapObj.lights.length + " lights.");

    for (var i = 0; i < mapObj.lights.length; ++i) {
        var obj = mapObj.lights[i];

        var sceneNode = new SceneNode();
        sceneNode.interactive = false;
        sceneNode.position = new Vector4(obj.position[0], obj.position[1], obj.position[2], 1);

        var light = new Light();
        light.type = obj.type;
        light.range = obj.range;
        // Enable this to see the range of lights
        // light.debugging = true;
        light.color = new Vector4(obj.color[0] / 255, obj.color[1] / 255, obj.color[2] / 255, 1);
        sceneNode.attachObject(light);

        scene.addNode(sceneNode);
    }

    print("Creating " + mapObj.particleSystems.length + " particle systems.");

    for (var i = 0; i < mapObj.particleSystems.length; ++i) {
        var obj = mapObj.particleSystems[i];

        var sceneNode = new SceneNode();
        sceneNode.interactive = false;
        sceneNode.position = new Vector4(obj.position[0], obj.position[1], obj.position[2], 1);

        var particleSystem = gameView.particleSystems.instantiate(obj.name);
        sceneNode.attachObject(particleSystem);

        scene.addNode(sceneNode);
    }

    gameView.centerOnWorld(mapObj.startPosition[0], mapObj.startPosition[2])
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
