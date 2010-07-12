
/**
 * Manages global numeric variables.
 */
var GlobalVars = {};

(function() {

    var values = {};

    /**
     * Changes the value of a global variable.
     *
     * @param id The id of the global variable.
     * @param value The new value of the global variable.
     */
    GlobalVars.set = function (id, value) {
        values[id] = value;
    };

    /**
     * Retrieves the value of a global variable.
     * @param id The id of the variable.
     * @returns The current value of the variable or 0, if it has never been set.
     */
    GlobalVars.get = function (id) {
        var value = values[id];
        
        if (value !== undefined)
            return value;
        else
            return 0;
    }

})();
