// Registry for render states.
var renderStates = {};
var globalRenderStateId = 0;

var currentTooltip = null;
var currentMouseOver = null;

/**
 * Base object for all prototypes
 * @constructor
 */
function BaseObject() {
}

BaseObject.prototype.scale = 100;
BaseObject.prototype.rotation = 0;
BaseObject.prototype.interactive = true;
BaseObject.prototype.drawBehindWalls = false;

/**
 * The number of hitpoints this object has. This is not the <i>current</i> amount of health, but rather the
 * maximum amount.
 */
BaseObject.prototype.hitPoints = 1;

/**
 * The number of temporary hit points this object has gained through spells or other means. If damage is taken,
 * this number is reduced until it hits zero, and the damage is avoided.
 */
BaseObject.prototype.temporaryHitPoints = 0;

/**
 * The damage sustained by this object or critter.
 */
BaseObject.prototype.damageTaken = 0;

/**
 * The subdual damage taken by this object. Certain objects may be immune to subdual damage (i.e. constructs,
 * or inanimate objects).
 */
BaseObject.prototype.subdualDamageTaken = 0;

BaseObject.prototype.getCurrentHp = function() {
    return this.hitPoints + this.temporaryHitPoints - this.damageTaken - this.subdualDamageTaken;
};

/**
 * Extracts the state of a mobile that needs to be saved.
 * Effectively, this returns a shallow copy of this object with
 * all runtime state (like render state ids) removed.
 */
BaseObject.prototype.persist = function() {
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
};

/**
 * Destroys this object.
 */
BaseObject.prototype.destroy = function() {
    this.disable();
    var index = this.map.mobiles.indexOf(this);
    if (index != -1) {
        this.map.mobiles.splice(index, 1);
    }
};

/**
 * Retrives a name for this object that can be presented to the user.
 *
 * @param unknown If true, the object is unknown, and if present, the unknown id should be returned.
 */
BaseObject.prototype.getName = function(unknown) {

    if (unknown && this.unknownDescriptionId !== undefined) {
        return translations.get('mes/description/' + this.unknownDescriptionId);
    }

    // A custom name overrides the prototype description id
    if (this.name !== undefined)
        return this.name;
    else
        return translations.get('mes/description/' + this.descriptionId);

};

BaseObject.prototype.getRenderState = function() {
    if (this.renderStateId === undefined)
        return null;

    return renderStates[this.renderStateId];
};

BaseObject.prototype.setRenderState = function(renderState) {
    if (this.renderStateId === undefined) {
        this.renderStateId = globalRenderStateId++;
    }

    renderStates[this.renderStateId] = renderState;
};

BaseObject.prototype.getWaterDepth = function() {
    // Auto move the object down if it's in a water-region
    if (this.prototype != 'StaticGeometry') {
        return Maps.currentMap.height(this.position);
    }

    return 0;
};

/**
 * Returns true if this object has a goal queued.
 */
BaseObject.prototype.isBusy = function() {
    return this.goal !== undefined;
};

/**
 * Sets the given goal for this object, this cancels any currently performing goals.
 *
 * @param goal The new goal to follow.
 */
BaseObject.prototype.setGoal = function(goal) {
    if (this.goal)
        this.goal.cancel(this);

    this.goal = goal;

    EventBus.notify(EventTypes.GoalStarted, this, goal);

    var obj = this;
    var updateTimer = function(time) {
        if (obj.goal === goal) {
            goal.advance(obj, time / 1000);

            if (!goal.isFinished()) {
                gameView.addVisualTimer(20, updateTimer);
            } else {
                EventBus.notify(EventTypes.GoalFinished, obj, goal);
                delete obj.goal;
            }
        }
    };
    gameView.addVisualTimer(20, updateTimer);
};

BaseObject.prototype.createRenderState = function() {
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

    var modelObj = gameView.models.load(this.model);

    var modelInstance = new ModelInstance(gameView.scene);
    modelInstance.model = modelObj;
    modelInstance.drawBehindWalls = this.drawBehindWalls;
    Equipment.addRenderEquipment(this, modelInstance);
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

    this.updateIdleAnimation();

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
};

BaseObject.prototype.removeRenderState = function() {
    // Remove from scene
    var renderState = this.getRenderState();
    if (renderState) {
        gameView.scene.removeNode(renderState.sceneNode);
        delete renderStates[this.renderStateId];
    }
    this.renderStateId = undefined;
};

BaseObject.prototype.disable = function() {
    if (this.disabled)
        return;

    this.removeRenderState();

    this.disabled = true;
};

BaseObject.prototype.enable = function() {
    if (!this.disabled)
        return;

    this.disabled = false;

    // Only do this if a map is loaded
    if (Maps.currentMap)
        this.createRenderState();
};

BaseObject.prototype.registerHandlers = function() {
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
};

BaseObject.prototype.getTooltipText = function() {
    var result = '<b>' + this.getName() + '</b>';
    if (this.damageTaken)
        result += qsTr("<br><i>Damage: %1</i>").arg(this.damageTaken);
    return result;
};

BaseObject.prototype.updateTooltip = function() {
    if (currentMouseOver === this && currentTooltip) {
        currentTooltip.text = this.getTooltipText();
    }
};

BaseObject.prototype.mouseEnter = function(event) {
    // During combat, the combat UI may override all interactions with objects
    if (Combat.isActive() && CombatUi.objectMouseEnter(this, event))
        return;

    var renderState = this.getRenderState();

    if (renderState)
        renderState.selectionCircle.hovering = true;

    var action = this.getDefaultAction(Party.getLeader());
    if (action) {
        gameView.currentCursor = action.cursor;
    }

    if (!currentTooltip) {
        currentTooltip = gameView.addGuiItem('interface/Tooltip.qml');
    }

    var screenPos = gameView.screenFromWorld(this.position);

    currentTooltip.text = this.getTooltipText();
    currentTooltip.x = screenPos[0] - currentTooltip.width / 2;
    currentTooltip.y = screenPos[1];
    currentTooltip.shown = true;
    currentMouseOver = this;
};

BaseObject.prototype.mouseLeave = function(event) {
    // During combat, the combat UI may override all interactions with objects
    if (Combat.isActive() && CombatUi.objectMouseLeave(this, event))
        return;

    var renderState = this.getRenderState();
    if (renderState)
        renderState.selectionCircle.hovering = false;

    // Always reset the cursor
    gameView.currentCursor = Cursors.Default;

    currentTooltip.shown = false;
    currentMouseOver = null;
};

BaseObject.prototype.setSelected = function(selected) {
    var renderState = this.getRenderState();
    if (renderState)
        renderState.selectionCircle.selected = selected;
};

BaseObject.prototype.clicked = function(event) {

    if (Combat.isActive()) {
        CombatUi.objectClicked(this, event);
        return;
    }

    /**
     * Shows a mobile info dialog if right-clicked on, selects the mobile
     * if left-clicked on.
     */
    if (event.button == Mouse.RightButton) {

        var renderState = this.getRenderState();
        if (renderState)
            showMobileInfo(this, renderState.modelInstance);

    } else if (event.button == Mouse.LeftButton) {

        var triggerer = Party.getLeader();

        var action = this.getDefaultAction(triggerer);

        if (!action) {
            print("No default action exists, skip action.");
            return;
        }

        var path = new Path(triggerer, this);

        if (!path.isEmpty()) {
            var movementGoal = new MovementGoal(path, false);
            var secondaryGoal = new ActionGoal(action);

            triggerer.setGoal(new GoalSequence(movementGoal, secondaryGoal));
        } else {
            print("No way of getting there -> skip.");
        }

    }

};

BaseObject.prototype.getReactionColor = function() {
    return [0.5, 0.5, 0.5]; // Neutral color
};

/**
 * Plays an animation once.
 * @param id The animation id, use constants provided by the Animation object.
 * @param stopHandler A callback that is called when the action was performed. This callback cannot be persisted,
 *          so the caller is responsible for restoring the state in case a savegame was loaded. If the
 *          object is not drawn on screen, the stop handler will be called automatically.
 */
BaseObject.prototype.playAnimation = function(id, stopHandler) {
    var renderState = this.getRenderState();

    if (renderState) {
        var model = renderState.modelInstance;

        if (!model.model.hasAnimation(id))
            id = 'unarmed_unarmed_' + id;

        model.playAnimation(id);

        if (stopHandler) {
            var eventHandler = function (animationId, canceled) {
                model.animationFinished.disconnect(eventHandler);
                stopHandler(animationId, canceled);
            };
            model.animationFinished.connect(eventHandler);
        }
    } else {
        if (stopHandler)
            stopHandler(); // Call immediately
    }
};

/**
 * If this object has a render-state, this sets the correct idle animation based
 * on this objedts state.
 */
BaseObject.prototype.updateIdleAnimation = function() {
    var renderState = this.getRenderState();

    if (!renderState || !renderState.modelInstance)
        return;

    var model = renderState.modelInstance.model;

    var idleAnimation;

    if (model.hasAnimation('item_idle')) {
        idleAnimation = 'item_idle';
    }

    if (renderState.modelInstance.idleAnimation != idleAnimation)
        renderState.modelInstance.idleAnimation = idleAnimation;

};

BaseObject.prototype.isUnconscious = function() {
    // TODO: Incorrect formula
    return (this.getCurrentHp() < 0);
};

/**
 * Deals damage to this object.
 * @param damage The amount of damage dealt.
 * @param source The damage source. May be null or undefined if damage is environmental.
 */
BaseObject.prototype.dealDamage = function(damage, source) {

    var alreadyDead = this.isUnconscious();

    var temporaryRemove = Math.min(damage, this.temporaryHitPoints);

    this.temporaryHitPoints -= temporaryRemove;
    damage -= temporaryRemove;

    this.damageTaken += damage;

    // Invoke death / disabled effects
    if (!alreadyDead) {
        if (this.damageTaken >= this.hitPoints) {

            // The event *can* prevent the death, upon which we will reduce
            // the damage taken until we remain @ 1 hp.
            if (this.OnDying) {
                if (LegacyScripts.OnDying(this.OnDying, this, source)) {
                    this.damageTaken = this.hitPoints - 1;
                    print("Death averted by script for " + this.id);
                    this.updateTooltip();
                    return;
                }
            }

            print("DEATH");
            this.playAnimation(Animations.Death);
            this.updateIdleAnimation();

            // Notify the event bus
            EventBus.notify(EventTypes.CritterDied, this, damage, source);
        } else {
            this.playAnimation(Animations.GetHitFront);
        }
    }

    this.updateTooltip();

};

/**
 * Deals subdual damage to this object.
 * @param damage The amount of damage dealt.
 * @param source The damage source, may be null or undefined if damage is environmental.
 */
BaseObject.prototype.dealSubdualDamage = function(damage, source) {
    this.subdualDamageTaken += damage;

    if (this.subdualDamageTaken + this.damageTaken >= this.temporaryHitPoints + this.hitPoints) {
        print("UNCONSCIOUS");
    }

    this.updateTooltip();
};

/**
 * Returns the default action that a given user (which is also an instance of baseobject) would perform
 * on this object.
 *
 * @param forUser The user for which the default action should be returned.
 */
BaseObject.prototype.getDefaultAction = function(forUser) {
    return null;
};

BaseObject.prototype.doubleClicked = function(event) {

    if (Combat.isActive()) {
        CombatUi.objectDoubleClicked(this, event);
    }

};
