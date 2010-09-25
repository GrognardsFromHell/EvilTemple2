var Action = function(name, description) {

    this.name = name;
    this.description = description;
    this.cursor = Cursors.Default;

};

Action.prototype.perform = function() {

};

var OpenContainerAction = function(container) {
    this.container = container;
    this.animation = Animations.UseObject;
};

OpenContainerAction.prototype = new Action('Open', 'Look inside this container');

OpenContainerAction.prototype.perform = function(critter) {
    showInventory(this.container);
};

var MeleeAttackAction = function(target) {
    this.target = target;
    this.animation = 'rattack';
};

MeleeAttackAction.prototype = new Action('Attack', 'Performs a melee attack');

MeleeAttackAction.prototype.perform = function(critter) {
    this.target.dealDamage(2, critter);
};
