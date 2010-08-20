var Class = function() {
};

/**
 * 0: Standard rules, roll the dice
 * 1: Maximum
 * 2: RPGA Rules, 1/2 maximum + 1
 */
var HpOnLevelUpSetting = 0;

var BaseAttackBoni = {
    Weak: [0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10],
    Medium: [0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 15],
    Strong: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]
};

var SavingThrows = {
    Weak: [0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6],
    Strong: [2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12]
};

(function() {

    /**
     * Returns a dice object that can be used to roll the hit die for this class on the given class level.
     *
     * For example all basic classes return the same hit die from this function. There are some prestige classes
     * however that have different hit die for different levels.
     *
     * The default implementation simply returns a dice object for the hitDie property of the object.
     *
     * @param classLevel The level of this class to get the hit die for. This must be greater than or equal to 1.
     */
    Class.prototype.getHitDie = function(classLevel) {
        return new Dice(this.hitDie);
    };

    /**
     * Returns the base attack bonus gained through having levels of this class.
     * For simple classes, this comes from three categories, strong, weak and medium, which are
     * stored in the baseAttackBonus property of the class.
     *
     * @param classLevel The class level to return the base attack bonus for.
     */
    Class.prototype.getBaseAttackBonus = function(classLevel) {
        return this.baseAttackBonus[classLevel - 1];
    };

    /**
     * Returns the base-bonus to the fortitude saving throw this class grants.
     * @param classLevel The number of levels of this class.
     */
    Class.prototype.getFortitudeSave = function(classLevel) {
        return this.fortitudeSave[classLevel - 1];
    };

    /**
     * Returns the base-bonus to the will saving throw this class grants.
     * @param classLevel The number of levels of this class.
     */
    Class.prototype.getWillSave = function(classLevel) {
        return this.willSave[classLevel - 1];
    };

    /**
     * Returns the base-bonus to the reflex saving throw this class grants.
     * @param classLevel The number of levels of this class.
     */
    Class.prototype.getReflexSave = function(classLevel) {
        return this.reflexSave[classLevel - 1];
    };

    Class.prototype.addClassLevel = function(character) {
        print("Giving level of " + this.id + " to " + character.id);

        // Is this the first level overall?
        var isFirst = character.getEffectiveCharacterLevel() == 0;
        var classLevel = character.getClassLevel(this.id);

        // Initialize the structure if necessary
        if (!classLevel) {
            character.classLevels.push({
                classId: this.id,
                count: 0
            });
            classLevel = character.getClassLevel(this.id);
        }

        var newClassLevel = classLevel.count + 1;
        classLevel.count = newClassLevel;

        var hpGained;
        var hitDie = this.getHitDie(newClassLevel);

        if (isFirst) {
            // Grant full hp on first character level
            hpGained = hitDie.getMaximum();
        } else {

            switch (HpOnLevelUpSetting) {
                case 0:
                    hpGained = this.getHitDie(newClassLevel).roll();
                    break;
                case 1:
                    hpGained = hitDie.getMaximum();
                    break;
                case 2:
                    hpGained = hitDie.getMaximum() / 2 + 1;
                    break;
                default:
                    throw "Unknown HpOnLevelUpSetting: " + HpOnLevelUpSetting;
            }
        }

        var hpConBonus = getAbilityModifier(character.constitution);

        // Initialize the array if it doesn't exist yet.
        if (!classLevel.hpGained)
            classLevel.hpGained = [];

        // Record the HP progression (separate by con bonus and actual hit die)
        classLevel.hpGained.push([hpGained, hpConBonus]);

        if (isFirst) {
            // Objects have a single hit point as the base if they have no classes. Overwrite this here.
            character.hitPoints = hpGained + hpConBonus;
        } else {
            character.hitPoints += hpGained + hpConBonus;
        }
    };

    /*
     Register the standard SRD classes.
     */
    function registerClasses() {
        Classes.register({
            id: 'barbarian',
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
            id: 'bard',
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
            id: 'cleric',
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
            id: 'druid',
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
            id: 'fighter',
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
            id: 'monk',
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
            id: 'paladin',
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
            id: 'ranger',
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
            id: 'rogue',
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
            id: 'sorcerer',
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
            id: 'wizard',
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
