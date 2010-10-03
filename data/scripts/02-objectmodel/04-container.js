/**
 * A container for items. Usually found in the world as chests.
 *
 * @constructor
 */
function Container() {
}

Container.prototype = new BaseObject;

Container.prototype.interactive = true;

/**
 * The amount of money contained in this container, in copper coins.
 */
Container.prototype.money = 0;

/**
 * Returns the default action that a given user (which is also an instance of baseobject) would perform
 * on this object.
 *
 * @param forUser The user for which the default action should be returned.
 */
Container.prototype.getDefaultAction = function(forUser) {
    return new OpenContainerAction(this);
};

function OpenContainerAction(container) {
    this.container = container;
    this.animation = Animations.UseObject;
    this.cursor = Cursors.Hand;
}

OpenContainerAction.prototype = new Action('Open', 'Look inside this container');

OpenContainerAction.prototype.perform = function(critter) {
    showInventory(this.container);
};
