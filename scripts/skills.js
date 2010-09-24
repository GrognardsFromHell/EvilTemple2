var Skills = {
};

var Skill = function() {
};

/**
 * Retrieves the synergy bonus given a set of skills. Defaults to 0.
 * 
 * @param skillRanks A map from skill ids to the corresponding ranks.
 * @param context The context in which this skill is used.
 */
Skill.prototype.getSynergyBonus = function(skillRanks, context) {
    return 0;
};

(function() {

    var skillsById = {};
    var skills = [];

    Skills.register = function(skillObj) {
        if (!skillObj.id)
            throw "Skill object has no id property.";

        if (skillsById[skillObj.id])
            throw "Skill with id " + skillObj.id + " is already registered.";

        var actualObj = new Skill;
        for (var k in skillObj) {
            if (skillObj.hasOwnProperty(k))
                actualObj[k] = skillObj[k];
        }

        skillsById[skillObj.id] = actualObj;
        skills.push(actualObj);
    };

    Skills.getById = function(id) {
        return skillsById[id];
    };

    Skills.getAll = function() {
        return skills.slice(0);
    };

})();
