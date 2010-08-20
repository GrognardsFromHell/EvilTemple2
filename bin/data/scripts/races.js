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

    /**
     * Registers several standard races. These may be moved to a separate data-file (JSON) later on.
     * On the other hand, these may also be extended with actual code, if not all properties of a race
     * can be described in a declarative way.
     */
    function registerDefaultRaces() {

        Races.register({
            id: 'human',
            playable: true,
            name: translations.get('mes/stat/2000'),
            description: translations.get('mes/stat/12000'),
            prototypeMale: '13000',
            prototypeFemale: '13001',
            heightMale: [147, 197],
            heightFemale: [135, 185],
            weightMale: [120, 200],
            weightFemale: [85, 165],
            landSpeed: 30
        });

        Races.register({
            id: 'dwarf',
            playable: true,
            name: translations.get('mes/stat/2001'),
            description: translations.get('mes/stat/12001'),
            prototypeMale: '13002',
            prototypeFemale: '13003',
            heightMale: [114, 129],
            heightFemale: [109, 134],
            weightMale: [130, 178],
            weightFemale: [100, 148],
            landSpeed: 20
        });

        Races.register({
            id: 'elf',
            playable: true,
            name: translations.get('mes/stat/2002'),
            description: translations.get('mes/stat/12002'),
            prototypeMale: '13004',
            prototypeFemale: '13005',
            heightMale: [135, 165],
            heightFemale: [135, 165],
            weightMale: [85, 121],
            weightFemale: [80, 116],
            landSpeed: 30
        });

        Races.register({
            id: 'gnome',
            playable: true,
            name: translations.get('mes/stat/2003'),
            description: translations.get('mes/stat/12003'),
            prototypeMale: '13006',
            prototypeFemale: '13007',
            heightMale: [91, 111],
            heightFemale: [86, 106],
            weightMale: [40, 48],
            weightFemale: [35, 43],
            landSpeed: 20
        });

        Races.register({
            id: 'halfelf',
            playable: true,
            name: translations.get('mes/stat/2004'),
            description: translations.get('mes/stat/12004'),
            prototypeMale: '13008',
            prototypeFemale: '13009',
            heightMale: [140, 180],
            heightFemale: [135, 175],
            weightMale: [100, 164],
            weightFemale: [80, 144],
            landSpeed: 30
        });

        Races.register({
            id: 'halforc',
            playable: true,
            name: translations.get('mes/stat/2005'),
            description: translations.get('mes/stat/12005'),
            prototypeMale: '130010',
            prototypeFemale: '13011',
            heightMale: [147, 208],
            heightFemale: [135, 195],
            weightMale: [150, 294],
            weightFemale: [110, 254],
            landSpeed: 30
        });

        Races.register({
            id: 'halfling',
            playable: true,
            name: translations.get('mes/stat/2006'),
            description: translations.get('mes/stat/12006'),
            prototypeMale: '130012',
            prototypeFemale: '13013',
            heightMale: [81, 111],
            heightFemale: [76, 106],
            weightMale: [30, 38],
            weightFemale: [25, 33],
            landSpeed: 20
        });

    }

    StartupListeners.add(registerDefaultRaces);

})();
