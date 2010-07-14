
var Map = function(id, filename) {
    if (!(this instanceof Map))
        throw "Constuct maps via the new keyword.";

    // Read the map once
    var mapObj = readJson(filename);

    this.id = id;
    this.filename = filename;
    this.name = mapObj.name;
    this.visited = false;

    // Load mobiles and link them to their prototypes
    this.mobiles = readJson(mapObj.mobiles);

    for (var i = 0; i < this.mobiles.length; ++i) {
        var mobile = this.mobiles[i];
        connectToPrototype(mobile);
        mobile.map = this;
    }
};

Map.prototype.entering = function(position) {
    print("Party is entering map " + this.name + " (" + this.id + ")");
};

Map.prototype.leaving = function(newMap, newPosition) {
    print("Party is leaving map " + this.name + " (" + this.id + ")");
};

Map.prototype.addMobile = function(mobile) {
    this.removeMobile(mobile); // Make sure there are no duplicates
    this.mobiles.push(mobile);
};

Map.prototype.removeMobile = function(mobile) {
    var idx = this.mobiles.indexOf(mobile);

    if (idx != -1)
        this.mobiles.splice(idx, 1);
};
