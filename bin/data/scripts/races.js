/**
 * Manager for races.
 */
var Races = {
};

/**
 * Default prototype for races.
 */
var Race = function(id) {
    this.id = id;
};

(function() {

    // This is kept locally to avoid accidental modification from outside this object
    var racesById = {}; // Map of all races by id    
    var races = []; // List of all races

    /**
     * Registers a race with this manager object.
     * @param race The race to register.
     */
    Races.register = function(race) {
        if (!race.id)
            throw "A race must have an id.";

        if (racesById[race.id])
            throw "Cannot register race " + race.id + " since it's already registered.";

        var raceObj = new Race(race.id);

        racesById[race.id] = race;
        races.push(race);

        print("Registered race " + race.name);
    };

    /**
     * Returns an array of all registered races.
     */
    Races.getAll = function() {
        return races.slice(0);
    };

    /**
     * Retrieves a race by its unique identifier.
     *
     * @param id The race's unique identifier.
     * @returns The race object or undefined.
     */
    Races.getById = function(id) {
        return racesById[id];
    };

})();
