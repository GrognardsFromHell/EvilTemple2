/**
 * Manages legacy dialog files.
 */
var LegacyDialog = {};

(function() {

    var dialogMapping;

    var dialogCache = {};

    function loadDialog(id) {
        var filename = dialogMapping[id];

        if (filename === undefined) {
            return undefined;
        }

        // TODO: Set a prototype on the resulting object?

        try {
            return eval('(' + readFile(filename) + ')');
        } catch(err) {
            print("Unable to read legacy dialog file " + filename);
            throw err;
        }
    }

    /**
     * Retrieves a legacy dialog object.
     * @param id The id of the legacy dialog.
     * @returns A legacy dialog object or undefined if the id is unknown.
     */
    function getDialog(id) {
        var obj = dialogCache[id];

        if (obj === undefined) {
            obj = loadDialog(id);
            dialogCache[id] = obj;
        }

        return obj;
    }    

    /**
     * Loads the legacy dialog model.
     */
    LegacyDialog.load = function() {
        dialogMapping = eval('(' + readFile('dialogs.js') + ')');

        // This counting method may be inaccurate, but at least it gives us some ballpark estimate
        var count = 0;
        for (var k in dialogMapping)
            count++;
        print("Loaded " + count + " legacy dialogs.");
    };

    /**
     * Retrieves a legacy dialog object by id.
     * @param id The id of the legacy dialog.
     */
    LegacyDialog.get = function(id) {
        return getDialog(id);
    };
})();
