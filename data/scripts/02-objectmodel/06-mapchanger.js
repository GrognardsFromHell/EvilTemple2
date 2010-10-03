/**
 * Objects that can be used to get to another map.
 *
 * @constructor
 */
function MapChanger() {
}

MapChanger.prototype = new BaseObject;

MapChanger.prototype.interactive = true;

MapChanger.prototype.getDefaultAction = function(forUser) {
    return new UseMapChangerAction(this);
};

function UseMapChangerAction(mapChanger) {
    if (!(this instanceof UseMapChangerAction))
        throw "Use the new keyword to construct actions.";

    this.combat = false;
    this.mapChanger = mapChanger;
    this.cursor = Cursors.Hand; 
}

UseMapChangerAction.prototype = new Action('Travel', 'Travel to another area');

UseMapChangerAction.prototype.perform = function(critter) {

    var mapChanger = this.mapChanger;

    var jumpPoint = jumppoints[mapChanger.teleportTarget];

    var newMap = Maps.mapsById[jumpPoint.map];

    if (!newMap) {
        print("JumpPoint " + mapChanger.teleportTarget + " links to unknown map: " + jumpPoint.map);
    } else {
        Maps.goToMap(newMap, jumpPoint.position);
    }

};
