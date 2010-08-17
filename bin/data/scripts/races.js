/**
 * Manager for races.
 */
var Races = {
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
            maleAppearance: {
                model: 'meshes/pcs/pc_human_male/pc_human_male.model',
                materials: {
                    HEAD: 'meshes/pcs/pc_human_male/head.xml',
                    CHEST: 'meshes/pcs/pc_human_male/chest.xml',
                    GLOVES: 'meshes/pcs/pc_human_male/hands.xml',
                    BOOTS: 'meshes/pcs/pc_human_male/feet.xml'
                }
            },
            femaleAppearance: {
                model: 'meshes/pcs/pc_human_female/pc_human_female.model',
                materials: {
                    HEAD: 'meshes/pcs/pc_human_female/head.xml',
                    CHEST: 'meshes/pcs/pc_human_female/chest.xml',
                    GLOVES: 'meshes/pcs/pc_human_female/hands.xml',
                    BOOTS: 'meshes/pcs/pc_human_female/feet.xml'
                }
            }
        });

        Races.register({
            id: 'dwarf',
            playable: true,
            name: translations.get('mes/stat/2001'),
            description: translations.get('mes/stat/12001')
        });

        Races.register({
            id: 'elf',
            playable: true,
            name: translations.get('mes/stat/2002'),
            description: translations.get('mes/stat/12002')
        });

        Races.register({
            id: 'gnome',
            playable: true,
            name: translations.get('mes/stat/2003'),
            description: translations.get('mes/stat/12003')
        });

        Races.register({
            id: 'halfelf',
            playable: true,
            name: translations.get('mes/stat/2004'),
            description: translations.get('mes/stat/12004')
        });

        Races.register({
            id: 'halforc',
            playable: true,
            name: translations.get('mes/stat/2005'),
            description: translations.get('mes/stat/12005')
        });

        Races.register({
            id: 'halfling',
            playable: true,
            name: translations.get('mes/stat/2006'),
            description: translations.get('mes/stat/12006')
        });

    }

    StartupListeners.add(registerDefaultRaces);

})();
