
/**
 * Describes the world map known to the party.
 */
var WorldMap = {};

(function() {

    var marked = {};

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
