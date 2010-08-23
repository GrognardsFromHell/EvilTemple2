var models; // Will be aliased to gameView.models

var jumppoints = {};

// Registry for render states.
var renderStates = {};
var globalRenderStateId = 0;

var editMode = false;

var tutorialMode = false; // Indicates that the tutorial is running.

var currentTooltip = null;

// Base object for all prototypes
//noinspection JSUnusedLocalSymbols
var BaseObject = {
    scale: 100,
    rotation: 0,
    interactive: true,
    drawBehindWalls: false,

    /**
     * The number of hitpoints this object has. This is not the <i>current</i> amount of health, but rather the
     * maximum amount.
     */
    hitPoints: 1,

    /**
     * The number of temporary hit points this object has gained through spells or other means. If damage is taken,
     * this number is reduced until it hits zero, and the damage is avoided.
     */
    temporaryHitPoints: 0,

    /**
     * The damage sustained by this object or critter.
     */
    damageTaken: 0,

    /**
     * The subdual damage taken by this object. Certain objects may be immune to subdual damage (i.e. constructs,
     * or inanimate objects).
     */
    subdualDamageTaken: 0,

    /**
     * Extracts the state of a mobile that needs to be saved.
     * Effectively, this returns a shallow copy of this object with
     * all runtime state (like render state ids) removed.
     */
    persist: function() {
        var result = {};

        for (var k in this) {
            // Skip prototype properties and functions
            if (!this.hasOwnProperty(k) || this[k] instanceof Function)
                continue;

            // Also skip volatile properties
            if (k == 'map' || k == 'renderStateId')
                continue;

            result[k] = this[k]; // Shallow-copy
        }

        return result;
    },

    /**
     * Destroys this object.
     */
    destroy: function() {
        this.disable();
        var index = this.map.mobiles.indexOf(this);
        if (index != -1) {
            this.map.mobiles.splice(index, 1);
        }
    },

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

    getWaterDepth: function() {
        // Auto move the object down if it's in a water-region
        if (this.prototype != 'StaticGeometry') {
            var depth = gameView.sectorMap.regionTag("water", this.position);

            if (depth !== undefined) {
                return depth;
            }
        }

        return 0;
    },

    createRenderState: function() {
        if (this.dontDraw || this.disabled)
            return;

        var sceneNode = gameView.scene.createNode();
        sceneNode.interactive = editMode || this.interactive;
        var pos = this.position.slice(0); // Create a copy
        pos[1] -= this.getWaterDepth();
        sceneNode.position = pos;
        sceneNode.rotation = rotationFromDegrees(this.rotation);
        var scale = this.scale / 100.0;
        sceneNode.scale = [scale, scale, scale];

        var modelObj = models.load(this.model);

        var modelInstance = new ModelInstance(gameView.scene);
        modelInstance.model = modelObj;
        modelInstance.drawBehindWalls = this.drawBehindWalls;
        updateEquipment(this, modelInstance);
        modelInstance.animationEvent.connect(this, handleAnimationEvent);
        if (this.prototype == 'StaticGeometry') {
            modelInstance.renderCategory = 'StaticGeometry';
        }

        // Store render state with the object
        var renderState = {
            modelInstance: modelInstance,
            sceneNode: sceneNode
        };
        this.setRenderState(renderState);

        if (editMode || this.interactive) {
            var selectionCircle = new SelectionCircle(gameView.scene, gameView.materials);
            renderState.selectionCircle = selectionCircle;

            if (this.radius !== undefined)
                selectionCircle.radius = this.radius;
            else
                selectionCircle.radius = 25;

            selectionCircle.color = this.getReactionColor();

            // TODO: This may be a bit slow for every interactive object.
            if (Selection.isSelected(this))
                selectionCircle.selected = true;

            sceneNode.attachObject(selectionCircle);

            this.registerHandlers(sceneNode, modelInstance);
        }
        sceneNode.attachObject(modelInstance);
    },

    removeRenderState: function() {
        // Remove from scene
        var renderState = this.getRenderState();
        if (renderState) {
            gameView.scene.removeNode(renderState.sceneNode);
            delete renderStates[this.renderStateId];
        }
        this.renderStateId = undefined;
    },

    disable: function() {
        if (this.disabled)
            return;

        this.removeRenderState();

        this.disabled = true;
    },

    enable: function() {
        if (!this.disabled)
            return;

        this.disabled = false;

        // Only do this if a map is loaded
        if (Maps.currentMap)
            this.createRenderState();
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

    mouseEnter: function(event) {
        var renderState = this.getRenderState();

        if (renderState)
            renderState.selectionCircle.hovering = true;

        if (this.OnDialog) {
            print("Setting cursor");
            gameView.currentCursor = 'art/interface/cursors/talk.png';
        }

        if (!currentTooltip) {
            currentTooltip = gameView.addGuiItem('interface/Tooltip.qml');
        }

        var screenPos = gameView.screenFromWorld(this.position);
        print(screenPos);

        currentTooltip.text = this.getName();
        currentTooltip.x = screenPos[0] - currentTooltip.width / 2;
        currentTooltip.y = screenPos[1];
        currentTooltip.shown = true;
    },

    mouseLeave: function(event) {
        var renderState = this.getRenderState();
        if (renderState)
            renderState.selectionCircle.hovering = false;
        if (this.OnDialog)
            gameView.currentCursor = 'art/interface/cursors/maincursor.png';
        currentTooltip.shown = false;
    },

    setSelected: function(selected) {
        var renderState = this.getRenderState();
        if (renderState)
            renderState.selectionCircle.selected = selected;
    },

    clicked: function(event) {
        /**
         * Shows a mobile info dialog if right-clicked on, selects the mobile
         * if left-clicked on.
         */
        if (event.button == Mouse.RightButton) {
            var renderState = this.getRenderState();
            if (renderState)
                showMobileInfo(this, renderState.modelInstance);
        } else if (event.button == Mouse.LeftButton) {
            if (editMode || Party.isMember(this)) {
                if (event.modifiers & KeyModifiers.Shift) {
                    if (Selection.isSelected(this)) {
                        Selection.remove([this]);
                    } else {
                        Selection.add([this]);
                    }
                } else {
                    Selection.clear();
                    Selection.add([this]);
                }
            }
        }
    },

    doubleClicked: function(event) {
    },

    getReactionColor: function() {
        return [0.5, 0.5, 0.5]; // Neutral color
    }
};

var Item = {
    __proto__: BaseObject,

    doubleClicked: function(event) {
        if (event.button == Mouse.LeftButton) {
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

    clicked: function(event) {
        if (event.button == Mouse.LeftButton) {
            showInventory(this);
        } else if (event.button == Mouse.RightButton) {
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
    clicked: function(event) {
        var renderState = this.getRenderState();

        if (event.button == Mouse.LeftButton) {
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
        } else if (event.button == Mouse.RightButton) {
            if (renderState)
                showMobileInfo(this, renderState.modelInstance);
        }
    }
};

var MapChanger = {
    __proto__: BaseObject,
    interactive: true,
    clicked: function(event) {
        var renderState = this.getRenderState();
        if (event.button == Mouse.LeftButton) {
            var jumpPoint = jumppoints[this.teleportTarget];

            var newMap = Maps.mapsById[jumpPoint.map];

            if (!newMap) {
                print("JumpPoint " + this.teleportTarget + " links to unknown map: " + jumpPoint.map);
            } else {
                Maps.goToMap(newMap, jumpPoint.position);
            }
        } else if (event.button == Mouse.RightButton) {
            if (renderState)
                showMobileInfo(this, renderState.modelInstance);
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
    classLevels: [],
    experiencePoints: 0,
    feats: [],

    /**
     * Checks whether this character has a certain feat.
     *
     * @param featInstance This is either the id of the feat (if the feat has no arguments)
     * or an array, which contains the feat id as the first element and subsequently all feat
     * arguments.
     */
    hasFeat: function(featInstance) {
        if (featInstance instanceof Array) {
            for (var i = 0; i < this.feats.length; ++i) {
                if (this.feats[i].equals(featInstance))
                    return true;
            }
            return false;
        }
        return this.feats.indexOf(featInstance) != -1;
    },

    /**
     * Returns the effective initiative bonus for this character, which includes the effect
     * from feats such as improved initiative and other influences.
     */
    getInitiativeBonus: function() {
        return getAbilityModifier(this.getEffectiveDexterity());
    },

    /**
     * Gets the effective land speed of this character. This uses the character's race as the
     * base land speed, and then applies any bonuses from the class or spells/effects.
     */
    getEffectiveLandSpeed: function() {
        var landSpeed = 30; // This is a fallback

        // Use the character's race as a base
        if (this.race) {
            var race = Races.getById(this.race);
            landSpeed = race.landSpeed;
        }

        // Allows a per-creature override of landspeed
        if (this.landSpeed) {
            landSpeed = this.landSpeed;
        }

        // Apply bonuses granted by the class.

        // TODO: Influence land-speed through encumbrance and armor

        return landSpeed;
    },

    /**
     * Returns the base attack bonus for this critter.
     */
    getBaseAttackBonus: function() {
        var bonus = 0;

        // This should probably be cached
        for (var i = 0; i < this.classLevels.length; ++i) {
            var classObj = Classes.getById(this.classLevels[i].classId);
            bonus += classObj.getBaseAttackBonus(this.classLevels[i].count);
        }

        return bonus;
    },

    /**
     * Gets the character's effective strength ability, which includes bonuses gained by spells and
     * other, temporary means.
     */
    getEffectiveStrength: function() {
        return this.strength;
    },

    /**
     * Gets the character's effective dexterity ability, which includes bonuses gained by spells and
     * other, temporary means.
     */
    getEffectiveDexterity: function() {
        return this.dexterity;
    },

    /**
     * Gets the character's effective constitution ability, which includes bonuses gained by spells and
     * other, temporary means.
     */
    getEffectiveConstitution: function() {
        return this.constitution;
    },

    /**
     * Gets the character's effective intelligence ability, which includes bonuses gained by spells and
     * other, temporary means.
     */
    getEffectiveIntelligence: function() {
        return this.intelligence;
    },

    /**
     * Gets the character's effective wisdom ability, which includes bonuses gained by spells and
     * other, temporary means.
     */
    getEffectiveWisdom: function() {
        return this.wisdom;
    },

    /**
     * Gets the character's effective charisma ability, which includes bonuses gained by spells and
     * other, temporary means.
     */
    getEffectiveCharisma: function() {
        return this.charisma;
    },

    getReflexSave: function() {
        var bonus = 0;

        // This should probably be cached
        for (var i = 0; i < this.classLevels.length; ++i) {
            var classObj = Classes.getById(this.classLevels[i].classId);
            bonus += classObj.getReflexSave(this.classLevels[i].count);
        }

        // Add dexterity modifier
        bonus += getAbilityModifier(this.getEffectiveDexterity());

        return bonus;
    },

    getWillSave: function() {
        var bonus = 0;

        // This should probably be cached
        for (var i = 0; i < this.classLevels.length; ++i) {
            var classObj = Classes.getById(this.classLevels[i].classId);
            bonus += classObj.getWillSave(this.classLevels[i].count);
        }

        // Add dexterity modifier
        bonus += getAbilityModifier(this.getEffectiveWisdom());

        return bonus;
    },

    getFortitudeSave: function() {
        var bonus = 0;

        // This should probably be cached
        for (var i = 0; i < this.classLevels.length; ++i) {
            var classObj = Classes.getById(this.classLevels[i].classId);
            bonus += classObj.getFortitudeSave(this.classLevels[i].count);
        }

        // Add dexterity modifier
        bonus += getAbilityModifier(this.getEffectiveConstitution());

        return bonus;
    },

    /**
     * Returns the effective level of this character. For now, this is the sum of all class-levels.
     */
    getEffectiveCharacterLevel: function() {
        var sum = 0;
        for (var i = 0; i < this.classLevels.length; ++i) {
            sum += this.classLevels[i].count;
        }
        return sum;
    },

    /**
     * Gets the number of levels this critter has of the given class.
     * @param classId The class's id.
     */
    getClassLevels: function(classId) {
        var sum = 0;
        for (var i = 0; i < this.classLevels.length; ++i) {
            if (this.classLevels[i].classId == classId)
                sum += this.classLevels[i].count;
        }
        return sum;
    },

    /**
     * Gets the object representing the levels of a certain class this character has.
     *
     * @param classId The class's id.
     * @returns null if this character doesnt have any levels of that class.
     */
    getClassLevel: function(classId) {
        for (var i = 0; i < this.classLevels.length; ++i) {
            if (this.classLevels[i].classId == classId)
                return this.classLevels[i];
        }
        return null;
    },

    joinedParty: function() {
        if (this.OnJoin)
            LegacyScripts.OnJoin(this.OnJoin, this);
    },

    leftParty: function() {
        if (this.OnDisband)
            LegacyScripts.OnDisband(this.OnDisband, this);
    }
};

var NonPlayerCharacter = {
    __proto__: Critter,

    /**
     * The amount of money held by this NPC, in copper coins.
     */
    money: 0,

    /**
     * The reaction. Starts @ 50, which is neutral.
     * Reaction table:
     * 60 and above - Good
     * 50 - Neutral (starting here)
     * 40 and less - Bad
     */
    reaction: 50,

    doubleClicked: function(event) {
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
    try {
        Maps.load();
    } catch (e) {
        throw "Error Loading Maps: " + e;
    }

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

    gameView.worldClicked.connect(function(event, worldPosition) {
        if (worldClickCallback != null) {
            var callback = worldClickCallback;
            worldClickCallback = null;
            callback(worldPosition);
            return;
        }

        // Clear the selection if right-clicked
        if (event.button == Mouse.RightButton) {
            Selection.clear();
            return;
        }

        var color;
        var material = gameView.sectorMap.regionTag("groundMaterial", worldPosition);

        if (!Selection.isEmpty()) {
            Selection.get().forEach(function(mobile) {
                walkTo(mobile, worldPosition);
            });
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

    gameView.worldDoubleClicked.connect(function(event, worldPosition) {
        print("Doubleclicked: " + objectToString(event));
        gameView.centerOnWorld(worldPosition);
    });
}

var animEventGameFacade = {
    particles: function(partSysId, proxyObject) {
        var renderState = proxyObject.obj.getRenderState();

        if (!renderState || !renderState.modelInstance || !renderState.modelInstance.model) {
            print("Called game.particles (animevent) for an object without rendering state: " + proxyObject.obj.id);
            return;
        }

        var particleSystem = gameView.particleSystems.instantiate(partSysId);
        particleSystem.modelInstance = renderState.modelInstance;
        renderState.sceneNode.attachObject(particleSystem);
    },
    sound_local_obj: function(soundId, proxyObject) {
        var filename = sounds[soundId];
        if (filename === undefined) {
            print("Unknown sound id: " + soundId);
            return;
        }

        print("Playing sound " + filename);
        var handle = gameView.audioEngine.playSoundOnce(filename, SoundCategory_Effect);
        handle.setPosition(proxyObject.obj.position);
        handle.setMaxDistance(1500);
        handle.setReferenceDistance(150);
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
        var material = gameView.sectorMap.regionTag("groundMaterial", this.obj.position);

        if (material === undefined)
            return;

        var sounds = footstepSounds[material];

        if (sounds === undefined) {
            print("Unknown material-type: " + material);
            return;
        }

        var sound = sounds[footstepCounter++ % sounds.length];

        var handle = gameView.audioEngine.playSoundOnce(sound, SoundCategory_Effect);
        handle.setPosition(this.obj.position);
        handle.setMaxDistance(1500);
        handle.setReferenceDistance(50);
    }
};

function handleAnimationEvent(type, content) {
    // Variable may be used by the eval call below.
    //noinspection UnnecessaryLocalVariableJS
    var game = animEventGameFacade;

    var anim_obj = {
        obj: this
    };
    anim_obj.__proto__ = animEventAnimObjFacade;

    if (type != 0) {
        print("Skipping unknown animation event type: " + type);
        return;
    }

    /*
     Python one-liners are in general valid javascript, so we directly evaluate them here.
     Eval has access to all local variables in this scope, so we can define game + anim_obj,
     which are the most often used by the animation events.
     */
    eval(content);
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
