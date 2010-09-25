var StandardSkills = {
    Appraise: 'appraise',
    Bluff: 'bluff',
    Concentration: 'concentration',
    Diplomacy: 'diplomacy',
    DisableDevice: 'disable-device',
    GatherInformation: 'gather-information',
    Heal: 'heal',
    Hide: 'hide',
    Intimidate: 'intimidate',
    Listen: 'listen',
    MoveSilently: 'move-silently',
    OpenLock: 'open-lock',
    Perform: 'perform',
    Search: 'search',
    SenseMotive: 'sense-motive',
    SleightOfHand: 'sleight-of-hand',
    Spellcraft: 'spellcraft',
    Spot: 'spot',
    Survival: 'survival',
    Tumble: 'tumble',
    UseMagicDevice: 'use-magic-device'
};

(function() {

    function registerSkills() {

        with (Skills) {
            register({
                id: StandardSkills.Appraise,
                name: translations.get('mes/skill/0'),
                description: translations.get('mes/skill/5000'),
                ability: Abilities.Intelligence
            });

            register({
                id: StandardSkills.Bluff,
                name: translations.get('mes/skill/1'),
                description: translations.get('mes/skill/5001'),
                ability: Abilities.Charisma
            });

            register({
                id: StandardSkills.Concentration,
                name: translations.get('mes/skill/2'),
                description: translations.get('mes/skill/5002'),
                ability: Abilities.Constitution
            });

            register({
                id: StandardSkills.Diplomacy,
                name: translations.get('mes/skill/3'),
                description: translations.get('mes/skill/5003'),
                ability: Abilities.Charisma,
                getSynergyBonus: function(skillRanks, context) {
                    var bonus = 0;
                    if (skillRanks[StandardSkills.Bluff] >= 5)
                        bonus += 2;
                    if (skillRanks[StandardSkills.SenseMotive] >= 5)
                        bonus += 2;
                    return bonus;
                }
            });

            register({
                id: StandardSkills.DisableDevice,
                name: translations.get('mes/skill/4'),
                description: translations.get('mes/skill/5004'),
                ability: Abilities.Dexterity
            });

            register({
                id: StandardSkills.GatherInformation,
                name: translations.get('mes/skill/5'),
                description: translations.get('mes/skill/5005'),
                ability: Abilities.Charisma
            });

            register({
                id: StandardSkills.Heal,
                name: translations.get('mes/skill/6'),
                description: translations.get('mes/skill/5006'),
                ability: Abilities.Wisdom
            });

            register({
                id: StandardSkills.Hide,
                name: translations.get('mes/skill/7'),
                description: translations.get('mes/skill/5007'),
                ability: Abilities.Dexterity
            });

            register({
                id: StandardSkills.Intimidate,
                name: translations.get('mes/skill/8'),
                description: translations.get('mes/skill/5008'),
                ability: Abilities.Charisma,
                getSynergyBonus: function(skillRanks, context) {
                    return skillRanks[StandardSkills.Bluff] >= 5 ? 2 : 0;
                }
            });

            register({
                id: StandardSkills.Listen,
                name: translations.get('mes/skill/9'),
                description: translations.get('mes/skill/5009'),
                ability: Abilities.Wisdom
            });

            register({
                id: StandardSkills.MoveSilently,
                name: translations.get('mes/skill/10'),
                description: translations.get('mes/skill/5010'),
                ability: Abilities.Dexterity
            });

            register({
                id: StandardSkills.OpenLock,
                name: translations.get('mes/skill/11'),
                description: translations.get('mes/skill/5011'),
                ability: Abilities.Dexterity
            });

            register({
                id: StandardSkills.Perform,
                name: translations.get('mes/skill/20'),
                description: translations.get('mes/skill/5020'),
                ability: Abilities.Charisma
            });

            register({
                id: StandardSkills.Search,
                name: translations.get('mes/skill/13'),
                description: translations.get('mes/skill/5013'),
                ability: Abilities.Intelligence
            });

            register({
                id: StandardSkills.SenseMotive,
                name: translations.get('mes/skill/14'),
                description: translations.get('mes/skill/5014'),
                ability: Abilities.Wisdom
            });

            register({
                id: StandardSkills.SleightOfHand,
                name: translations.get('mes/skill/12'),
                description: translations.get('mes/skill/5012'),
                ability: Abilities.Dexterity,
                getSynergyBonus: function(skillRanks, context) {
                    return skillRanks[StandardSkills.Bluff] >= 5 ? 2 : 0;
                }
            });

            register({
                id: StandardSkills.Spellcraft,
                name: translations.get('mes/skill/15'),
                description: translations.get('mes/skill/5015'),
                ability: Abilities.Intelligence,
                getSynergyBonus: function(skillRanks, context) {
                    // TODO: Context is non-formalized
                    if (context == 'DecipherScroll') {
                        return skillRanks[StandardSkills.UseMagicDevice] >= 5 ? 2 : 0;
                    } else {
                        return 0;
                    }
                }
            });

            register({
                id: StandardSkills.Spot,
                name: translations.get('mes/skill/16'),
                description: translations.get('mes/skill/5016'),
                ability: Abilities.Wisdom
            });

            register({
                id: StandardSkills.Survival,
                name: translations.get('mes/skill/19'),
                description: translations.get('mes/skill/5019'),
                ability: Abilities.Wisdom,
                getSynergyBonus: function(skillRanks, context) {
                    // TODO: Context is non-formalized
                    if (context == 'Tracking') {
                        return skillRanks[StandardSkills.Search] >= 5 ? 2 : 0;
                    } else {
                        return 0;
                    }
                }
            });

            register({
                id: StandardSkills.Tumble,
                name: translations.get('mes/skill/17'),
                description: translations.get('mes/skill/5017'),
                ability: Abilities.Dexterity
            });

            register({
                id: StandardSkills.UseMagicDevice,
                name: translations.get('mes/skill/18'),
                description: translations.get('mes/skill/5018'),
                ability: Abilities.Charisma,
                getSynergyBonus: function(skillRanks, context) {
                    // TODO: Context is non-formalized
                    // Rather: for targets that are scrolls
                    if (context == 'Scroll') {
                        return skillRanks[StandardSkills.UseMagicDevice] >= 5 ? 2 : 0;
                    } else {
                        return 0;
                    }
                }
            });

        }

    }

    StartupListeners.add(registerSkills, 'd20-skills', []);

})();
