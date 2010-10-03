/**
 * Doors that can be opened or closed.
 * @constructor
 */
function Portal() {
}

Portal.prototype = new BaseObject;

Portal.prototype.interactive = true;

Portal.prototype.opened = false; // All doors are shut by default

Portal.prototype.updateIdleAnimation = function() {

    var renderState = this.getRenderState();

    if (!renderState || !renderState.modelInstance)
        return;

    var model = renderState.modelInstance.model;

    var idleAnimation = 'item_idle';

    if (this.opened && model.hasAnimation('open_idle'))
        idleAnimation = 'open_idle';

    if (renderState.modelInstance.idleAnimation != idleAnimation)
        renderState.modelInstance.idleAnimation = idleAnimation;
};

Portal.prototype.open = function(critter) {
    if (!this.opened) {
        print("Opening door.");

        var renderState = this.getRenderState();
        renderState.modelInstance.playAnimation('open', false);

        this.opened = true;
        this.updateIdleAnimation();
    }
};

Portal.prototype.close = function(critter) {
    if (this.opened) {
        print("Closing door.");

        var renderState = this.getRenderState();
        renderState.modelInstance.playAnimation('close', false);

        this.opened = true;
        this.updateIdleAnimation();
    }
};

Portal.prototype.getDefaultAction = function(forUser) {
    if (this.opened) {
        return new ClosePortalAction(this);
    } else {
        return new OpenPortalAction(this);
    }
};

function OpenPortalAction(portal) {
    this.portal = portal;
    this.animation = Animations.UseObject;
}

OpenPortalAction.prototype = new Action('Open', 'Open the door');

OpenPortalAction.prototype.perform = function(critter) {
    this.portal.open(critter);
};

function ClosePortalAction(portal) {
    this.portal = portal;
    this.animation = Animations.UseObject;
}

ClosePortalAction.prototype = new Action('Close', 'Close the door');

ClosePortalAction.prototype.perform = function(critter) {
    this.portal.close(critter);
};
