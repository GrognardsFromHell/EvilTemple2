/**
 * Registry for character classes. The corresponding class object will be registered
 * using its unique identifier (a string, e.g. "bard").
 */
var Classes = {};

(function() {

    var classesById = {};
    var classes = [];

    Classes.register = function(classObj) {
        if (!classObj.id)
            throw "Class object has no id property.";

        if (classesById[classObj.id])
            throw "Class with id " + classObj.id + " is already registered.";

        var actualObj = new Class;
        for (var k in classObj) {
            if (classObj.hasOwnProperty(k))
                actualObj[k] = classObj[k];
        }

        classesById[classObj.id] = actualObj;
        classes.push(actualObj);
    };

    Classes.getById = function(id) {
        return classesById[id];
    };

    Classes.getAll = function() {
        return classes.slice(0);
    };

})();
