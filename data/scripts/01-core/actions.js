function Action(name, description) {
    /**
     * A human readable name for this action.
     */
    this.name = name;

    /**
     * A human readable description of this action.
     */
    this.description = description;

    /**
     * The cursor to show for selecting this action. This is either the cursor
     * shown for objects that have this action as their default action, or
     * if a target needs to be selected for this action.
     */
    this.cursor = Cursors.Default;

    /**
     * Allows the use of this action during combat.
     */
    this.combat = true;
}

Action.prototype.perform = function() {
    throw "perform must be overriden for actions.";
};

function MeleeAttackAction(target) {
    if (!(this instanceof MeleeAttackAction))
        throw "Use the new keyword to construct actions";

    this.target = target;
    this.animation = Animations.AttackRight;
    this.cursor = Cursors.Sword;
}

MeleeAttackAction.prototype = new Action('Attack', 'Performs a melee attack');

MeleeAttackAction.prototype.perform = function(critter) {
    this.target.dealDamage(2, critter);
};
