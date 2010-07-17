
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
        }
    };

})();
