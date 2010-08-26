/**
 * Provides a registry for feats.
 */
var Feats = {};

var Feat = function() {
    this.unique = true;
};

/**
 * Returns a human-readable name for an instance of the feat.
 * This is particularily useful for displaying the name of a feat that has arguments.
 *
 * So in a character sheet, Weapon Focus (Longsword) gets listed instead of Weapon Focus.
 */
Feat.prototype.getName = function(instance) {
    if (!this.argument) {
        return this.name;
    }

    var instanceOption = instance[1];

    var result = this.name + ' (';
    // Find the corresponding argument and use it's text property
    var values = this.argument.values;
    for (var j = 0; j < values.length; ++j) {
        if (values[j].id == instanceOption) {
            result += values[j].text;
        }
    }
    result += ')';

    return result;
};

/**
 * Creates a feat argument.
 *
 * @param name The name of the argument. This is for display purposes to the user.
 * @param description The description part of the argument. This is displayed to the user when he is queried
 * for the argument upon choosing the feat.
 * @param values The possible values for the argument. As objects with at least an id and text property each.
 */
var FeatArgument = function(name, description, values) {
    return {
        name: name,
        description: description,
        values: values
    }
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
