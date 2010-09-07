/*
 Defines various goals that characters can perform.

 Goals must provide the following methods:
 advance(character, timespan)
 cancel(character)
 isFinished(character)

 In addition, goals *must* be serializable using JSON.
 */

var Goal = function() {
};

Goal.prototype.advance = function(critter, time) {
};

Goal.prototype.cancel = function(critter) {
};

Goal.prototype.isFinished = function(critter) {
    return true;
};

Goal.prototype.animationAction = function(critter, event) {
    print("[GOAL] Received animation event for " + critter.getName() + " " + event);
};

/**
 * Creates a sequence of goals that itself is a goal.
 * This may be used for a scenario like this:
 * First: Walk to container
 * Then: Open container
 */
var GoalSequence = function() {
    this.currentGoal = 0;
    this.goals = [];
    for (var i = 0; i < arguments.length; ++i) {
        this.goals.push(arguments[i]);
    }
};

GoalSequence.prototype = new Goal;

GoalSequence.prototype.advance = function(character, timespan) {
    if (this.isFinished())
        return;

    this.goals[this.currentGoal].advance(character, timespan);

    if (this.goals[this.currentGoal].isFinished())
        this.currentGoal++;
};

GoalSequence.prototype.cancel = function(character) {
    if (this.isFinished())
        return;

    this.goals[this.currentGoal].cancel(character);
    this.currentGoal = this.goals.length;
};

GoalSequence.prototype.isFinished = function(character) {
    return this.currentGoal >= this.goals.length;
};

/**
 * Forwards the animation event to the active goal of this sequence.
 * If the sequence is already finished, the event is discarded.
 *
 * @param critter The critter for which the animation is playing.
 * @param event The animation event that occured.
 */
Goal.prototype.animationAction = function(critter, event) {
    if (this.isFinished())
        return;

    this.goals[this.currentGoal].animationAction(critter, event);
};

/**
 * Creates a goal that will move a character to a new position along a given path.
 * @param path The path (an array of positions) that will be taken by the character.
 * @param walking True if the character should walk instead of run.
 */
var MovementGoal = function(path, walking) {
    this.path = path;
    this.walking = walking;
    this.currentPathNode = 0;
    this.path = path;
    this.driven = 0;
};

MovementGoal.prototype = new Goal;

MovementGoal.prototype.beginPathSegment = function(critter) {

    var from = this.path[this.currentPathNode];
    var to = this.path[this.currentPathNode + 1];

    var renderState = critter.getRenderState();
    var sceneNode = renderState.sceneNode;

    // Build a directional vector from the start pointing to the target
    var x = to[0] - from[0];
    var y = to[1] - from[1];
    var z = to[2] - from[2];

    var l = Math.sqrt((x * x) + (y * y) + (z * z));

    print("Pathlength: " + l + " Direction: " + x + "," + y + "," + z);

    this.length = l;
    this.direction = [x, y, z];

    // Normal view-direction
    var nx = 0;
    var ny = 0;
    var nz = -1;

    x /= l;
    y /= l;
    z /= l;

    var rot = Math.acos(nx * x + ny * y + nz * z);

    if (x > 0) {
        rot = - rot;
    }

    var needed = rad2deg(rot) - critter.rotation;

    if (needed > 1) {
        // TODO: We do need to rotate "visibly" using the correct turning animation.
    }

    critter.rotation = rad2deg(rot);
    sceneNode.rotation = [0, Math.sin(rot / 2), 0, Math.cos(rot / 2)];
};

MovementGoal.prototype.advance = function(critter, time) {

    var renderState = critter.getRenderState();

    // TODO: Movement goals should work even without render states.
    var sceneNode = renderState.sceneNode;
    var modelInstance = renderState.modelInstance;

    if (!this.initialized) {
        this.initialized = true;
        modelInstance.playAnimation('unarmed_unarmed_run', true);
        this.speed = modelInstance.model.animationDps('unarmed_unarmed_run'); // Pixel per second

        this.beginPathSegment(critter);
    }

    var driven = this.speed * time;
    modelInstance.elapseDistance(driven);
    this.driven += driven;

    var completion = this.driven / this.length;

    if (completion < 1) {
        var pos = this.path[this.currentPathNode];
        var dx = this.direction[0] * completion;
        var dz = this.direction[2] * completion;

        var position = [pos[0] + dx, 0, pos[2] + dz];
        critter.position = position.slice(0); // Assign a copy, since position will be modified further
        position[1] -= critter.getWaterDepth(); // Depth is DISPLAY ONLY
        sceneNode.position = position;
    } else {
        this.currentPathNode += 1;
        if (this.currentPathNode + 1 >= this.path.length) {
            modelInstance.stopAnimation();
            return;
        }
        this.driven = 0;

        this.beginPathSegment(critter);
    }
};

MovementGoal.prototype.cancel = function(character) {
    var renderState = character.getRenderState();
    renderState.modelInstance.stopAnimation();
};

MovementGoal.prototype.isFinished = function(character) {
    return this.currentPathNode + 1 > this.path.length;
};

var MeleeAttackGoal = function(target) {
    this.target = target;
};

MeleeAttackGoal.prototype = new Goal;

MeleeAttackGoal.prototype.advance = function(critter, time) {

    if (!this.animated) {
        var goal = this;
        critter.playAnimation('rattack', function() {
            /*
             Always perform the action, even if the animation has no corresponding event, in this case
             we perform the action after the animation has stopped playing.
             */
            if (!goal.actionPerformed) {
                goal.performAction(critter);
            }
            goal.finished = true;
        });
        this.animated = true;
    }

};

MeleeAttackGoal.prototype.cancel = function(critter) {
    var renderState = critter.getRenderState();
    renderState.modelInstance.stopAnimation();
};

MeleeAttackGoal.prototype.isFinished = function(critter) {
    return this.finished;
};

MeleeAttackGoal.prototype.animationAction = function(critter, event) {
    this.performAction(critter);
};

MeleeAttackGoal.prototype.performAction = function(critter, event) {

    this.target.dealDamage(2, critter);

    this.actionPerformed = true;
};
