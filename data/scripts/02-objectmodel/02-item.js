/**
 * The base prototype for all items in the game.
 * @constructor
 */
function Item() {
}

Item.prototype = new BaseObject;

Item.prototype.doubleClicked = function(event) {
    if (Combat.isActive()) {
        CombatUi.objectDoubleClicked(this, event);
        return;
    }

    if (event.button == Mouse.LeftButton) {
        if (this.OnUse) {
            LegacyScripts.OnUse(this.OnUse, this);
        }
    }
};
