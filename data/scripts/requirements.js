var FeatRequirement = function(feat) {
    if (!(this instanceof FeatRequirement))
        return new FeatRequirement(feat);

    // TODO: This is rather hacky at the moment, until it's been decided how to handle this
    if (arguments.length > 1) {
        feat = [];
        for (var i = 0; i < arguments.length; ++i) {
            feat[i] = arguments[i];
        }
    }

    this.feat = feat;
};

FeatRequirement.prototype.isMet = function(character, contextArgs) {

    // If there's a "SameArgument" in the args, check the context for the necessary arguments.
    if (this.feat instanceof Array) {
        // Check whether we need to make a "faux" instance to check for
        if (this.feat.indexOf(FeatRequirement.SameArgument) != -1) {
            var fauxInstance = [];
            for (var i = 0; i < this.feat; ++i) {
                if (this.feat[i] === FeatRequirement.SameArgument) {
                    fauxInstance.push(contextArgs[i - 1]);
                } else {
                    fauxInstance.push(this.feat[i]);
                }
            }
            return character.hasFeat(fauxInstance);
        }
    }

    return character.hasFeat(this.feat);
};

/**
 * Returns the name of the feat that is required.
 */
FeatRequirement.prototype.toString = function() {
    var feat;
    if (this.feat instanceof Array) {
        feat = Feats.getById(this.feat[0]);
    } else {
        feat = Feats.getById(this.feat);
    }
    return feat.getName(this.feat);
};

/**
 * This special object can be used to indicate that the requirement uses the same argument as the context in which
 * the requirement is used. This can only be used as the requirement for feats.
 *
 * Example: Greater spell focus requires the spell focus feat with the same arguments as the greater spell focus.
 * In turn, this means that the greater spell focus arguments are implied by the existing spell focus feats.
 */
FeatRequirement.SameArgument = '$$$same-argument$$$';

/**
 * Specifies that the character must have at least the given caster level in the given type.
 * @param type One of the constants accessible through this object. Any, Divine, or Arcane.
 * @param level The minimum caster level that is required.
 */
var CasterLevelRequirement = function(type, level) {
    if (!(this instanceof CasterLevelRequirement))
        return new CasterLevelRequirement(type, level);

    this.magicType = type;
    this.level = level;
};

CasterLevelRequirement.Any = 'any';
CasterLevelRequirement.Divine = 'divine';
CasterLevelRequirement.Arcane = 'arcane';

CasterLevelRequirement.prototype.isMet = function(character, contextArgs) {
    // TODO: Implement
    return false;
};

CasterLevelRequirement.prototype.toString = function() {
    return "Caster level " + this.level;
};

/**
 * Specifies that the character must have at least a certain ability score.
 * @param ability The ability to check. Use constants from the Abilities object.
 * @param score
 */
var AbilityRequirement = function(ability, score) {
    if (!(this instanceof AbilityRequirement))
        return new AbilityRequirement(ability, score);

    this.ability = ability;
    this.minimum = score;
};

AbilityRequirement.prototype.isMet = function(character, contextArgs) {
    return character[this.ability] >= this.minimum;
};

AbilityRequirement.prototype.toString = function() {
    var result = '';
    switch (this.ability) {
        case Abilities.Strength:
            result = 'Str';
            break;
        case Abilities.Dexterity:
            result = 'Dex';
            break;
        case Abilities.Constitution:
            result = 'Con';
            break;
        case Abilities.Wisdom:
            result = 'Wis';
            break;
        case Abilities.Intelligence:
            result = 'Int';
            break;
        case Abilities.Charisma:
            result = 'Cha';
            break;
    }

    return result + ' ' + this.minimum;
};

/**
 * Requires a minimum base attack bonus from the character.
 * @param bab The minimum BAB to require.
 */
var BaseAttackBonusRequirement = function(bab) {
    if (!(this instanceof BaseAttackBonusRequirement))
        return new BaseAttackBonusRequirement(bab);

    this.minimum = bab;
};

BaseAttackBonusRequirement.prototype.isMet = function(character, contextArgs) {
    return character.getBaseAttackBonus() >= this.minimum;
};

BaseAttackBonusRequirement.prototype.toString = function() {
    return "Base attack bonus " + bonusToString(this.minimum);
};

/**
 * Requires that the character has the ability to turn (or rebuke) undead.
 */
var TurnOrRebukeRequirement = function() {
    if (!(this instanceof TurnOrRebukeRequirement))
        return new TurnOrRebukeRequirement();
};

TurnOrRebukeRequirement.prototype.isMet = function(character, contextArgs) {
    // TODO: Implement
    return false;
};

TurnOrRebukeRequirement.prototype.toString = function() {
    return "Ability to turn or rebuke undead.";
};

/**
 * A requirement that only applies under certain conditions.
 * @param condition The condition. This is an opaque specification and depends on the context.
 * @param requirement The actual requirement that is wrapped by this object.
 */
var ConditionalRequirement = function(condition, requirement) {
    if (!(this instanceof ConditionalRequirement))
        return new ConditionalRequirement(condition, requirement);

    this.condition = condition;
    this.requirement = requirement;
};

ConditionalRequirement.prototype.isMet = function(character, contextArgs) {
    if (contextArgs == this.condition) {
        return this.requirement.isMet(character, contextArgs);
    }
    return true;
};

ConditionalRequirement.prototype.toString = function() {
    return this.requirement.toString() + " (" + this.condition + ")";
};

/**
 * Requires at least the given level of a certain class.
 * @param classId The id of the class that is required.
 * @param level The minimum level of class that is required.
 */
var ClassLevelRequirement = function(classId, level) {
    if (!(this instanceof ClassLevelRequirement))
        return new ClassLevelRequirement(classId, level);

    this.classId = classId;
    this.minimum = level;
};

ClassLevelRequirement.prototype.isMet = function(character, contextArgs) {
    // TODO: Implement
    return true;
};

ClassLevelRequirement.prototype.toString = function() {
    return this.classId + " " + this.minimum;
};

/**
 * This requirement applies to feats only and specifies that the weapon argument of the feat (i.e. Improved Critical)
 * must be a weapon the character is proficient with.
 */
var ProficientWithWeaponRequirement = function() {
    if (!(this instanceof ProficientWithWeaponRequirement))
        return new ProficientWithWeaponRequirement();
};

ProficientWithWeaponRequirement.prototype.isMet = function(character, contextArgs) {
    // TODO: Implement
    return false;
};

ProficientWithWeaponRequirement.prototype.toString = function() {
    return qsTr("Proficient with selected weapon.");
};

/**
 * Require the character to be at least a certain level.
 * @param level The minimum required level.
 */
var CharacterLevelRequirement = function(level) {
    if (!(this instanceof CharacterLevelRequirement))
        return new CharacterLevelRequirement(level);

    this.minimum = level;
};

CharacterLevelRequirement.prototype.isMet = function(character, contextArgs) {
    // TODO: Implement
    return false;
};

CharacterLevelRequirement.prototype.toString = function() {
    return "Character level " + this.minimum;
};

/**
 * Requires that the character has the ability to wildshape.
 */
var WildShapeAbilityRequirement = function() {
    if (!(this instanceof WildShapeAbilityRequirement))
        return new WildShapeAbilityRequirement();
};

WildShapeAbilityRequirement.prototype.isMet = function(character, contextArgs) {
    // TODO: Implement
    return false;
};

WildShapeAbilityRequirement.prototype.toString = function() {
    return "Ability to wildshape.";
};

/**
 * Requires that the character has a minimum number of ranks in a skill.
 * @param skill
 * @param rank
 */
var SkillRequirement = function(skill, rank) {
    if (!(this instanceof SkillRequirement))
        return new SkillRequirement(skill, rank);

    this.skillId = skill;
    this.minimum = rank;
};

SkillRequirement.prototype.isMet = function(character, contextArgs) {
    // TODO: Implement
    return false;
};

SkillRequirement.prototype.toString = function() {
    return this.skillId + " Rank " + this.rank;
};
