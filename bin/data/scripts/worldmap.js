/**
 * Describes the world map known to the party.
 */
var WorldMap = {};

(function() {

    /**
     * Currently this is a slight hack to always enable hommlet...
     * It seems like this would be a better thing to do in the vignette->homlett transition
     */
    var marked = {'hommlet': true};

    // Callbacks that will be notified about newly marked areas
    var newAreaCallbacks = [];

    function notifyMarkedNewArea(id) {
        newAreaCallbacks.forEach(function (callback) {
            callback[1].call(callback[0], id);
        });
    }

    /**
     * Adds a listener that will be called when a new area has been marked on the worldmap.
     *
     * @param callback The function to call when a new area is marked. It will get the new area id
     *                 as it's only parameter.
     * @param thisObj  The this object for the function call.
     */
    WorldMap.addMarkedNewAreaListener = function(callback, thisObj) {
        newAreaCallbacks.push([thisObj, callback]);
    };

    /**
     * Removes a listener from this object.
     * @param callback The callback function of the listener.
     * @param thisObj The this object of the listener. 
     */
    WorldMap.removeMarkedNewAreaListener = function(callback, thisObj) {
        var removed = newAreaCallbacks.splice(newAreaCallbacks.indexOf[thisObj, callback], 1);
        return removed.length == 1;
    };

    /**
     * Gets a list of all areas that are marked on the world map.
     */
    WorldMap.getMarkedAreas = function() {
        var result = [];
        for (var k in marked)
            result.push(k);
        return result;
    };

    /**
     * Checks whether an area has been marked on the world map.
     * @param id The id of the area.
     */
    WorldMap.isMarked = function(id) {
        return marked[id] === true;
    };

    /**
     * Mark an area as visible on the world map.
     * @param id The id of the area.
     */
    WorldMap.mark = function(id) {
        if (marked[id] === undefined) {
            print("Marking a previously unknown area on the worldmap: " + id);
            marked[id] = true;
            notifyMarkedNewArea(id);
        }
    };

})();
