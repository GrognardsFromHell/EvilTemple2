/**
 * Constants for the ids of the standard d20 srd classes.
 */
var StandardClasses = {
    Barbarian: 'barbarian',
    Bard: 'bard',
    Cleric: 'cleric',
    Druid: 'druid',
    Fighter: 'fighter',
    Monk: 'monk',
    Paladin: 'paladin',
    Ranger: 'ranger',
    Rogue: 'rogue',
    Sorcerer: 'sorcerer',
    Wizard: 'wizard'
};

/**
 * Commonly used base attack bonus progressions.
 */
var BaseAttackBoni = {
    Weak: [0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10],
    Medium: [0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 15],
    Strong: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]
};

/**
 * Commonly used saving throw progressions.
 */
var SavingThrows = {
    Weak: [0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6],
    Strong: [2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12]
};

(function() {

    /*
     Register the standard SRD classes.
     */
    function registerClasses() {
        Classes.register({
            id: StandardClasses.Barbarian,
            name: translations.get('mes/stat/7'),
            description: translations.get('mes/stat/13000'),
            hitDie: 'd12',
            requirements: [
                {
                    type: 'alignment',
                    exclusive: [Alignment.LawfulGood, Alignment.LawfulNeutral, Alignment.LawfulEvil]
                }
            ],
            skillPoints: 4,
            baseAttackBonus: BaseAttackBoni.Strong,
            fortitudeSave: SavingThrows.Strong,
            reflexSave: SavingThrows.Weak,
            willSave: SavingThrows.Weak
        });

        Classes.register({
            id: StandardClasses.Bard,
            name: translations.get('mes/stat/8'),
            description: translations.get('mes/stat/13001'),
            hitDie: 'd6',
            requirements: [
                {
                    type: 'alignment',
                    exclusive: [Alignment.LawfulGood, Alignment.LawfulNeutral, Alignment.LawfulEvil]
                }
            ],
            skillPoints: 6,
            baseAttackBonus: BaseAttackBoni.Medium,
            fortitudeSave: SavingThrows.Weak,
            reflexSave: SavingThrows.Strong,
            willSave: SavingThrows.Strong
        });

        Classes.register({
            id: StandardClasses.Cleric,
            name: translations.get('mes/stat/9'),
            description: translations.get('mes/stat/13002'),
            hitDie: 'd6',
            requirements: [],
            skillPoints: 2,
            baseAttackBonus: BaseAttackBoni.Medium,
            fortitudeSave: SavingThrows.Strong,
            reflexSave: SavingThrows.Weak,
            willSave: SavingThrows.Strong
        });

        Classes.register({
            id: StandardClasses.Druid,
            name: translations.get('mes/stat/10'),
            description: translations.get('mes/stat/13003'),
            hitDie: 'd8',
            requirements: [
                {
                    type: 'alignment',
                    inclusive: [Alignment.NeutralGood, Alignment.LawfulNeutral, Alignment.TrueNeutral,
                        Alignment.ChaoticNeutral, Alignment.NeutralEvil]
                }
            ],
            skillPoints: 4,
            baseAttackBonus: BaseAttackBoni.Medium,
            fortitudeSave: SavingThrows.Strong,
            reflexSave: SavingThrows.Weak,
            willSave: SavingThrows.Strong
        });

        Classes.register({
            id: StandardClasses.Fighter,
            name: translations.get('mes/stat/11'),
            description: translations.get('mes/stat/13004'),
            hitDie: 'd10',
            requirements: [],
            skillPoints: 2,
            baseAttackBonus: BaseAttackBoni.Strong,
            fortitudeSave: SavingThrows.Strong,
            reflexSave: SavingThrows.Weak,
            willSave: SavingThrows.Weak
        });

        Classes.register({
            id: StandardClasses.Monk,
            name: translations.get('mes/stat/12'),
            description: translations.get('mes/stat/13005'),
            hitDie: 'd8',
            requirements: [
                {
                    type: 'alignment',
                    inclusive: [Alignment.LawfulGood, Alignment.LawfulNeutral, Alignment.LawfulEvil]
                }
            ],
            skillPoints: 4,
            baseAttackBonus: BaseAttackBoni.Medium,
            fortitudeSave: SavingThrows.Strong,
            reflexSave: SavingThrows.Strong,
            willSave: SavingThrows.Strong
        });

        Classes.register({
            id: StandardClasses.Paladin,
            name: translations.get('mes/stat/13'),
            description: translations.get('mes/stat/13006'),
            hitDie: 'd10',
            requirements: [
                {
                    type: 'alignment',
                    inclusive: [Alignment.LawfulGood]
                }
            ],
            skillPoints: 2,
            baseAttackBonus: BaseAttackBoni.Strong,
            fortitudeSave: SavingThrows.Strong,
            reflexSave: SavingThrows.Weak,
            willSave: SavingThrows.Weak
        });

        Classes.register({
            id: StandardClasses.Ranger,
            name: translations.get('mes/stat/14'),
            description: translations.get('mes/stat/13007'),
            hitDie: 'd8',
            requirements: [],
            skillPoints: 6,
            baseAttackBonus: BaseAttackBoni.Strong,
            fortitudeSave: SavingThrows.Strong,
            reflexSave: SavingThrows.Strong,
            willSave: SavingThrows.Weak
        });

        Classes.register({
            id: StandardClasses.Rogue,
            name: translations.get('mes/stat/15'),
            description: translations.get('mes/stat/13008'),
            hitDie: 'd6',
            requirements: [],
            skillPoints: 8,
            baseAttackBonus: BaseAttackBoni.Medium,
            fortitudeSave: SavingThrows.Weak,
            reflexSave: SavingThrows.Strong,
            willSave: SavingThrows.Weak
        });

        Classes.register({
            id: StandardClasses.Sorcerer,
            name: translations.get('mes/stat/16'),
            description: translations.get('mes/stat/13009'),
            hitDie: 'd4',
            requirements: [],
            skillPoints: 2,
            baseAttackBonus: BaseAttackBoni.Weak,
            fortitudeSave: SavingThrows.Weak,
            reflexSave: SavingThrows.Weak,
            willSave: SavingThrows.Strong
        });

        Classes.register({
            id: StandardClasses.Wizard,
            name: translations.get('mes/stat/17'),
            description: translations.get('mes/stat/13010'),
            hitDie: 'd4',
            requirements: [],
            skillPoints: 2,
            baseAttackBonus: BaseAttackBoni.Weak,
            fortitudeSave: SavingThrows.Weak,
            reflexSave: SavingThrows.Weak,
            willSave: SavingThrows.Strong
        });

    }

    StartupListeners.add(registerClasses);

})();
