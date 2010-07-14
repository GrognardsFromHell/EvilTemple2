
var Map = function(id, mobiles) {
    if (!(this instanceof Map))
        throw "Constuct maps via the new keyword.";

    this.id = id;
    this.mobiles = [];

    for (var i = 0; i < mobiles.length; ++i) {
        var obj = {};
        obj.map = id;
        obj.__proto__ = mobiles[i]; // To serialize this, all we need is the mobile GUID
        this.mobiles.push(obj);
    }
};
