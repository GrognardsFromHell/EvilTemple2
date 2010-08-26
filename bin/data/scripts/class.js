var Class = function() {
    /**
     * A table that defines the bonus feats granted by this class at a given class-level.
     * Bonus feats are still subject to be chosen by the player, they are not all granted
     * automatically.
     */
    this.bonusFeats = {};
};

/**
 * 0: Standard rules, roll the dice
 * 1: Maximum
 * 2: RPGA Rules, 1/2 maximum + 1
 */
var HpOnLevelUpSetting = 0;

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

    Class.prototype.getClassSkills = function(character) {
        return this.classSkills;
    };

    /**
     * Returns the bonus feats a character may choose at a given class level.
     *
     * The default implementation will simply return the entry from this objects
     * bonusFeats table or null if no such entry exists for the given classlevel.
     *
     * @param classLevel The class level at which the bonus feats apply.
     * @returns An object with two properties:
     * count - The number of feats a character may choose from the list of allowed feats.
     * feats - The list of feat id's from which a character may choose bonus feats.
     * If no bonus feats are available at that level, null is returned.
     */
    Class.prototype.getBonusFeatsAtLevel = function(classLevel) {
        return this.bonusFeats[classLevel] ? this.bonusFeats[classLevel] : null;
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

})();
