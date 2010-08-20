/**
 * Provides a registry for feats.
 */
var Feats = {};

var Feat = function() {
};

(function() {

    var featsById = {};
    var feats = [];

    Feats.register = function(featObj) {
        print("Registering feat " + featObj.id + " (" + featObj.name + ")");

        if (!featObj.id)
            throw "Feat object has no id property.";

        if (featsById[featObj.id])
            throw "Feat with id " + featObj.id + " is already registered.";

        var actualObj = new Feat;
        for (var k in featObj) {
            if (featObj.hasOwnProperty(k))
                actualObj[k] = featObj[k];
        }

        featsById[featObj.id] = actualObj;
        feats.push(actualObj);
    };

    Feats.getById = function(id) {
        return featsById[id];
    };

    Feats.getAll = function() {
        return feats.slice(0);
    };

})();
