var Deities = {};

var Deity = function() {
};

(function() {

    var deitiesById = {};
    var deities = [];

    Deities.register = function(deityObj) {
        if (!deityObj.id)
            throw "Deity object has no id property.";

        if (deitiesById[deityObj.id])
            throw "Deity with id " + deityObj.id + " is already registered.";

        var actualObj = new Deity;
        for (var k in deityObj) {
            if (deityObj.hasOwnProperty(k))
                actualObj[k] = deityObj[k];
        }

        deitiesById[deityObj.id] = actualObj;
        deities.push(actualObj);
    };

    Deities.getById = function(id) {
        return deitiesById[id];
    };

    Deities.getAll = function() {
        return deities.slice(0);
    };

})();
