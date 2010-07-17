var models; // Will be aliased to gameView.models

var jumppoints = {};

var currentSelection = null;

// Registry for render states.
var renderStates = {};
var globalRenderStateId = 0;

// Base object for all prototypes
var BaseObject = {
    scale: 100,
    rotation: 0,
    interactive: true,
    drawBehindWalls: false,

    /**
     * Retrives a name for this object that can be presented to the user.
     *
     * @param unknown If true, the object is unknown, and if present, the unknown id should be returned.
     */
    getName: function(unknown) {

        if (unknown && this.unknownDescriptionId !== undefined) {
            return translations.get('mes/description/' + this.unknownDescriptionId);
        }

        // A custom name overrides the prototype description id
        if (this.name !== undefined)
            return this.name;
        else
            return translations.get('mes/description/' + this.descriptionId);

    },

    getRenderState: function() {
        if (this.renderStateId === undefined)
            return null;

        return renderStates[this.renderStateId];
    },

    setRenderState: function(renderState) {
        if (this.renderStateId === undefined) {
            this.renderStateId = globalRenderStateId++;
        }

        renderStates[this.renderStateId] = renderState;
    },

    registerHandlers: function() {
        var renderState = this.getRenderState();

        var selectionCircle = renderState.selectionCircle;
        selectionCircle.mouseReleased.connect(this, this.clicked);
        selectionCircle.mouseDoubleClicked.connect(this, this.doubleClicked);
        selectionCircle.mouseEnter.connect(this, this.mouseEnter);
        selectionCircle.mouseLeave.connect(this, this.mouseLeave);

        var modelInstance = renderState.modelInstance;

        modelInstance.mouseReleased.connect(this, this.clicked);
        modelInstance.mouseDoubleClicked.connect(this, this.doubleClicked);
        modelInstance.mouseEnter.connect(this, this.mouseEnter);
        modelInstance.mouseLeave.connect(this, this.mouseLeave);
    },

    mouseEnter: function(buttons) {
        var renderState = this.getRenderState();
        if (renderState)
            renderState.selectionCircle.hovering = true;
    },

    mouseLeave: function(buttons) {
        var renderState = this.getRenderState();
        if (renderState)
            renderState.selectionCircle.hovering = false;
    },

    setSelected: function(selected) {
        var renderState = this.getRenderState();
        if (renderState)
            renderState.selectionCircle.selected = selected;
    },

    clicked: function(button, buttons) {
        /**
         * Shows a mobile info dialog if right-clicked on, selects the mobile
         * if left-clicked on.
         */
        if (button == Mouse.RightButton) {
            var renderState = this.getRenderState();
            if (renderState)
                showMobileInfo(this, renderState.modelInstance);
        } else if (button == Mouse.LeftButton) {
            if (currentSelection != null)
                currentSelection.setSelected(false);

            currentSelection = this;

            this.setSelected(true);
        }
    },

    doubleClicked: function() {
    },

    getReactionColor: function() {
        return [1, 1, 0]; // Neutral color
    }
};

var Item = {
    __proto__: BaseObject,

    doubleClicked: function(button) {
        if (button == Mouse.LeftButton) {
            if (this.OnUse) {
                LegacyScripts.OnUse(this.OnUse, this);
            }
        }
    }
};

var Scenery = {
    __proto__: Item
};

var Container = {
    __proto__: Item,
    interactive: true,

    /**
     * The amount of money contained in this container, in copper coins.
     */
    money: 0,

    clicked: function(button) {
        if (button == Mouse.LeftButton) {
            showInventory(this);
        } else if (button == Mouse.RightButton) {
            var renderState = this.getRenderState();
            if (renderState)
                showMobileInfo(this, renderState.modelInstance);
        }
    }
};

var Portal = {
    __proto__: Item,
    interactive: true,
    open: false, // All doors are shut by default
    clicked: function(button) {
        var renderState = this.getRenderState();

        if (button == Mouse.LeftButton) {
            if (this.open) {
                print("Closing door.");
                renderState.modelInstance.playAnimation('close', false);
                renderState.modelInstance.idleAnimation = 'item_idle';
            } else {
                print("Opening door.");
                if (!renderState.modelInstance.playAnimation('open', false)) {
                    print("Unable to play open animation for door.");
                }
                renderState.modelInstance.idleAnimation = 'open_idle';
            }
            this.open = !this.open;
        } else if (button == Mouse.RightButton) {
            if (renderState)
                showMobileInfo(this, renderState.modelInstance);
        }
    }
};

var MapChanger = {
    __proto__: BaseObject,
    interactive: true,
    clicked: function() {
        var jumpPoint = jumppoints[this.teleportTarget];

        var newMap = Maps.mapsById[jumpPoint.map];

        if (!newMap) {
            print("JumpPoint " + this.teleportTarget + " links to unknown map: " + jumpPoint.map);
        } else {
            Maps.goToMap(newMap, [jumpPoint.x, 0, jumpPoint.z]);
        }
    }
};

var Critter = {
    __proto__: BaseObject,
    concealed: false,
    getReactionColor: function() {
        if (this.killsOnSight) {
            return [1, 0, 0]; // Friendly
        } else {
            return [0.33, 1, 0]; // Friendly
        }
    },
    getReaction: function() {
        return Reaction.Neutral;
    },
    drawBehindWalls: true,
    killsOnSight: false,

    joinedParty: function() {
        // Remove the mobile from the map's list of mobiles
        if (this.map) {
            this.map.removeMobile(this);
        }

        if (this.OnJoin)
            LegacyScripts.OnJoin(this.OnJoin, this);
    },

    leftParty: function() {
        if (this.OnDisband)
            LegacyScripts.OnDisband(this.OnDisband, this);

        // Add the mobile back to the current map's list of mobiles
        if (this.map) {
            this.map.addMobile(this);
        }
    }
};

var NonPlayerCharacter = {
    __proto__: Critter,

    /**
     * The amount of money held by this NPC, in copper coins.
     */
    money: 0,

    doubleClicked: function() {
        /*
         If there's a OnDialog script associated, trigger the event in the legacy script system.
         */
        if (this.OnDialog)
            LegacyScripts.OnDialog(this.OnDialog, this);
    },

    getReaction: function() {
        // TODO: This should keep track of the reaction value and return the status accordingly
        return Reaction.Neutral;
    }
};

var PlayerCharacter = {
    __proto__: Critter
};

var prototypes;
var sounds;

function startup() {
    models = gameView.models;

    print("Calling startup hooks.");
    StartupListeners.call();

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

    print("Loading subsystems.");
    LegacyScripts.load();
    LegacyDialog.load();
    Maps.load();

    // Assign the prototype of each loaded prototype
    for (var i in prototypes) {
        var type = prototypes[i].type;

        if (type == 'MapChanger')
            prototypes[i].__proto__ = MapChanger;
        else if (type == 'Portal')
            prototypes[i].__proto__ = Portal;
        else if (type == 'Container')
            prototypes[i].__proto__ = Container;
        else if (type == 'Scenery')
            prototypes[i].__proto__ = Scenery;
        else if (type == 'NonPlayerCharacter')
            prototypes[i].__proto__ = NonPlayerCharacter;
        else if (type == 'PlayerCharacter')
            prototypes[i].__proto__ = PlayerCharacter;
        else
            prototypes[i].__proto__ = BaseObject;
    }

    // Accept clicks from the game view
    setupWorldClickHandler();

    print("Showing main menu.");
    MainMenuUi.show();
}

var worldClickCallback = null;

function selectWorldTarget(callback) {
    worldClickCallback = callback;
}

function setupWorldClickHandler() {
    var firstClick = undefined;

    gameView.worldClicked.connect(function(button, buttons, worldPosition) {
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

        var text;
        if (firstClick === undefined) {
            text = "1st @ " + Math.floor(worldPosition[0]) + "," + Math.floor(worldPosition[2]) + " (" + material + ")";
            gameView.scene.addTextOverlay(worldPosition, text, [0.9, 0.9, 0.9, 0.9]);
            firstClick = worldPosition;
        } else {
            var inLos = gameView.sectorMap.hasLineOfSight(firstClick, worldPosition);
            if (inLos) {
                color = [0.2, 0.9, 0.2, 0.9];
            } else {
                color = [0.9, 0.2, 0.2, 0.9];
            }

            text = "2nd @ " + Math.floor(worldPosition[0]) + "," + Math.floor(worldPosition[2]) + " (" + material + ")";
            gameView.scene.addTextOverlay(worldPosition, text, color);

            var path = gameView.sectorMap.findPath(firstClick, worldPosition);

            print("Found path of length: " + path.length);

            var sceneNode = gameView.scene.createNode();

            for (var i = 0; i < path.length; ++i) {
                if (i + 1 < path.length) {
                    var line = new LineRenderable(gameView.scene);
                    line.addLine(path[i], path[i + 1]);
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

    gameView.worldDoubleClicked.connect(function(button, buttons, worldPosition) {
        print("Doubleclicked: " + button);
        gameView.centerOnWorld(worldPosition);
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
            print("Unknown sound id: " + soundId);
            return;
        }

        print("Playing sound " + filename);
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

        var handle = gameView.audioEngine.playSoundOnce(sound, SoundCategory_Effect);
        handle.setPosition(this.sceneNode.position);
        handle.setMaxDistance(1500);
        handle.setReferenceDistance(50);
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
    if (obj.dontDraw || obj.disabled)
        return;

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
    var renderState = {
        modelInstance: modelInstance,
        sceneNode: sceneNode
    };
    obj.setRenderState(renderState);

    if (obj.interactive) {
        var selectionCircle = new SelectionCircle(scene, gameView.materials);
        renderState.selectionCircle = selectionCircle;

        if (obj.radius !== undefined)
            selectionCircle.radius = obj.radius;

        selectionCircle.color = obj.getReactionColor();

        sceneNode.attachObject(selectionCircle);

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
