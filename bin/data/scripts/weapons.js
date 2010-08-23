/**
 * Describes weapon types and their properties. These types can be found in the D20 SRD under the "Weapons"
 * category.
 */
var Weapons = {
};

var Weapon = function() {
};

(function() {

    var weaponsById = {};
    var weapons = [];

    Weapons.register = function(weaponsObj) {
        print("Registering weapon " + weaponsObj.id + " (" + weaponsObj.name + ")");

        if (!weaponsObj.id)
            throw "Weapon object has no id property.";

        if (weaponsById[weaponsObj.id])
            throw "Weapon with id " + weaponsObj.id + " is already registered.";

        var actualObj = new Weapon;
        for (var k in weaponsObj) {
            if (weaponsObj.hasOwnProperty(k))
                actualObj[k] = weaponsObj[k];
        }

        weaponsById[weaponsObj.id] = actualObj;
        weapons.push(actualObj);
    };

    Weapons.getById = function(id) {
        return weaponsById[id];
    };

    Weapons.getAll = function() {
        return weapons.slice(0);
    };

    Weapons.getByCategory = function(category) {
        return weapons.filter(function (weapon) {
            return weapon.category == category;
        });
    };

})();
