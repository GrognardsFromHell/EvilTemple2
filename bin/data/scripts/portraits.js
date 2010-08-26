var Portraits = {
    fallbackId: '0'
};

var Portrait = function() {
};

Portrait.Large = 'large';
Portrait.Medium = 'medium';
Portrait.Small = 'small';

(function() {

    var portraitsById = {};
    var portraits = [];

    Portraits.register = function(portraitsObj) {
        print("Registering portrait " + portraitsObj.id + " (" + portraitsObj.name + ")");

        if (portraitsObj.id === null || portraitsObj.id === undefined)
            throw "Portrait object has no id property.";

        if (portraitsById[portraitsObj.id])
            throw "Portrait with id " + portraitsObj.id + " is already registered.";

        var actualObj = new Portrait;
        for (var k in portraitsObj) {
            if (portraitsObj.hasOwnProperty(k))
                actualObj[k] = portraitsObj[k];
        }

        portraitsById[portraitsObj.id] = actualObj;
        portraits.push(actualObj);
    };

    Portraits.getById = function(id) {
        return portraitsById[id];
    };

    /**
     * Retrieves a portrait image file by size and returns a placeholder if the portrait or size doesn't
     * exist.
     *
     * @param id The portrait id.
     * @param size The portrait size. (Optional, will default to Medium)
     */
    Portraits.getImage = function(id, size) {
        if (!size)
            size = Portrait.Medium;

        var record = portraitsById[id];

        if (!record) {
            record = portraits[Portraits.fallbackId]; // Fallback portrait
        }

        if (!record)
            throw "Fallback portrait " + Portraits.fallbackId + " couldn't be accessed.";

        return 'art/interface/portraits/' + record[size];
    };

    Portraits.getAll = function() {
        return portraits.slice(0);
    };
    
    function loadPortraits() {
        print("Loading portraits...");
        eval('(' + readFile('portraits.js') + ')').forEach(function (portrait) {
            Portraits.register(portrait);
        });
    }

    StartupListeners.add(loadPortraits, 'toee-portraits', []);

})();
