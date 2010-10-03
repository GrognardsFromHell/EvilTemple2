/**
 * The prototype for non-player characters.
 *
 * @constructor
 */
function NonPlayerCharacter() {
}

NonPlayerCharacter.prototype = new Critter();

/**
 * The amount of money held by this NPC, in copper coins.
 */
NonPlayerCharacter.prototype.money = 0;

/**
 * The reaction. Starts @ 50, which is neutral.
 * Reaction table:
 * 60 and above - Good
 * 50 - Neutral (starting here)
 * 40 and less - Bad
 */
NonPlayerCharacter.prototype.reaction = 50;

NonPlayerCharacter.prototype.doubleClicked = function(event) {

    if (Combat.isActive()) {
        CombatUi.objectDoubleClicked(this, event);
        return;
    }

    /*
     If there's a OnDialog script associated, trigger the event in the legacy script system.
     */
    if (this.OnDialog)
        LegacyScripts.OnDialog(this.OnDialog, this);
};

NonPlayerCharacter.prototype.getReaction = function() {
    // TODO: This should keep track of the reaction value and return the status accordingly
    if (this.killsOnSight)
        return Reaction.Hostile;
    else
        return Reaction.Neutral;
};
