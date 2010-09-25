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
};

MovementGoal.prototype = new Goal;

MovementGoal.prototype.updateRotation = function(critter) {

    // Normal view-direction
    var rotOrigin = [0, 0, -1];
    var direction = this.path.currentDirection();

    var rot = Math.acos(V3.dot(rotOrigin, direction));
    if (direction[0] > 0) {
        rot = - rot;
    }

    var needed = rad2deg(rot) - critter.rotation;

    if (needed > 1) {
        // TODO: We do need to rotate "visibly" using the correct turning animation.
    }

    var renderState = critter.getRenderState();
    var sceneNode = renderState.sceneNode;

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
        var animation = this.walking ? 'unarmed_unarmed_walk' : 'unarmed_unarmed_run';
        modelInstance.playAnimation(animation, true);
        this.speed = modelInstance.model.animationDps(animation); // Movement speed is dictated by the animation data
        this.updateRotation(critter);
    }

    if (!this.path.isEmpty()) {
        this.updateRotation(critter);

        var driven = this.speed * time;
        modelInstance.elapseDistance(driven);

        var position = this.path.advance(driven);
        critter.position = position.slice(0); // Assign a copy, since position will be modified further

        position[1] = critter.getWaterDepth(); // Depth is DISPLAY ONLY
        sceneNode.position = position;
    }

    if (this.path.isEmpty())
        modelInstance.stopAnimation();
};

MovementGoal.prototype.cancel = function(character) {
    var renderState = character.getRenderState();
    renderState.modelInstance.stopAnimation();
};

MovementGoal.prototype.isFinished = function(character) {
    return this.path.isEmpty();
};

/**
 * Moves a critter in range of an object.
 * @param target The object to move into range of.
 * @param range The range to move into.
 */
var MoveInRangeGoal = function(target, range) {
    this.target = target;
    this.range = range;
};

MoveInRangeGoal.prototype.advance = function(critter, time) {

    var distanceToTarget = distance(critter.position, target.position);
    var gapRemaining = distanceToTarget - target.radius - critter.radius;

    if (gapRemaining < range) {
        this.finished = true;
    } else {
        // Calculate a path to the target and advance on it until we are within range.
        var path = Maps.currentMap.findPath(critter, this.target.position);

        if (path.length < 2) {
            this.finished = true; // Pathing is impossible.
            return;
        }

        // Advance on the path

    }

};

MoveInRangeGoal.prototype.cancel = function(character) {
    var renderState = character.getRenderState();
    renderState.modelInstance.stopAnimation();
};

MoveInRangeGoal.prototype.isFinished = function(character) {
    /**
     * Do NOT test if the character is in range here. The walk-animation may still be playing if
     * the caller never calls advance again (in case this method returns true).
     */
    return this.finished;
};

var ActionGoal = function(action) {
    this.action = action;
};

ActionGoal.prototype = new Goal;

ActionGoal.prototype.advance = function(critter, time) {

    if (!this.action.animation) {
        this.action.perform(critter);
        this.actionPerformed = true;
        this.finished = true;
        return;
    }

    if (!this.animated) {
        var goal = this;
        critter.playAnimation(this.action.animation, function() {
            /*
             Always perform the action, even if the animation has no corresponding event, in this case
             we perform the action after the animation has stopped playing.
             */
            if (!goal.actionPerformed) {
                this.action.perform(critter);
                this.actionPerformed = true;
            }
            goal.finished = true;
        });
        this.animated = true;
    }

};

ActionGoal.prototype.cancel = function(critter) {
    if (this.animation) {
        var renderState = critter.getRenderState();
        renderState.modelInstance.stopAnimation();
    }
};

ActionGoal.prototype.isFinished = function(critter) {
    return this.finished;
};

ActionGoal.prototype.animationAction = function(critter, event) {
    this.action.perform(critter);
    this.actionPerformed = true;
};
