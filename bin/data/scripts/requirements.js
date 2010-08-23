var FeatRequirement = function(feat) {
    // TODO: This is rather hacky at the moment, until it's been decided how to handle this
    if (arguments.length > 1) {
        feat = [];
        for (var i = 0; i < arguments.length; ++i) {
            feat[i] = arguments[i];
        }
    }

    return {
        type: 'feat',
        feat: feat
    }
};

/**
 * This special object can be used to indicate that the requirement uses the same arguments as the context in which
 * the requirement is used. This can only be used as the requirement for feats.
 *
 * Example: Greater spell focus requires the spell focus feat with the same arguments as the greater spell focus.
 * In turn, this means that the greater spell focus arguments are implied by the existing spell focus feats.
 */
FeatRequirement.SameArguments = '$$$same-arguments$$$';

/**
 * Specifies that the character must have at least the given caster level in the given type.
 * @param type One of the constants accessible through this object. Any, Divine, or Arcane.
 * @param level The minimum caster level that is required.
 */
var CasterLevelRequirement = function(type, level) {
    return {
        type: 'casterLevel',
        magicType: type,
        level: level
    }
};

CasterLevelRequirement.Any = 'any';
CasterLevelRequirement.Divine = 'divine';
CasterLevelRequirement.Arcane = 'arcane';

/**
 * Specifies that the character must have at least a certain ability score.
 * @param ability The ability to check. Use constants from the Abilities object.
 * @param score
 */
var AbilityRequirement = function(ability, score) {
    return {
        type: 'ability',
        ability: ability,
        minimum: score
    }
};

/**
 * Requires a minimum base attack bonus from the character.
 * @param bab The minimum BAB to require.
 */
var BaseAttackBonusRequirement = function(bab) {
    return {
        type: 'bab',
        minimum: bab
    }
};

/**
 * Requires that the character has the ability to turn (or rebuke) undead.
 */
var TurnOrRebukeRequirement = function() {
    return {
        type: 'turnOrRebuke'
    };
};

/**
 * A requirement that only applies under certain conditions.
 * @param condition The condition. This is an opaque specification and depends on the context.
 * @param requirement The actual requirement that is wrapped by this object.
 */
var ConditionalRequirement = function(condition, requirement) {
    return {
        type: 'conditional',
        condition: condition,
        requirement: requirement
    }
};

/**
 * Requires at least the given level of a certain class.
 * @param classId The id of the class that is required.
 * @param level The minimum level of class that is required.
 */
var ClassLevelRequirement = function(classId, level) {
    return {
        type: 'classLevel',
        classId: classId,
        minimum: level
    }
};

/**
 * This requirement applies to feats only and specifies that the weapon argument of the feat (i.e. Improved Critical)
 * must be a weapon the character is proficient with.
 */
var ProficientWithWeaponRequirement = function() {
    return {
        type: 'proficientWithWeapon'
    }
};

/**
 * Require the character to be at least a certain level.
 * @param level The minimum required level.
 */
var CharacterLevelRequirement = function(level) {
    return {
        type: 'charLevel',
        minimum: level
    };
};

/**
 * Requires that the character has the ability to wildshape.
 */
var WildShapeAbilityRequirement = function() {
    return {
        type: 'wildshape'
    }
};

/**
 * Requires that the character has a minimum number of ranks in a skill.
 * @param skill
 * @param rank
 */
var SkillRequirement = function(skill, rank) {
    return {
        type: 'skill',
        skillId: skill,
        minimum: rank
    }
};
