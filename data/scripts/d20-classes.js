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
 * Defines the feat-ids for all the bonus-feats a fighter may take.
 */
var FighterBonusFeats = [];

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

        // Sets the standard bonus feats available to a fighter.
        [
            StandardFeats.BlindFight,
            StandardFeats.CombatExpertise,
            StandardFeats.ImprovedDisarm,
            StandardFeats.ImprovedFeint,
            StandardFeats.ImprovedTrip,
            StandardFeats.WhirlwindAttack,
            StandardFeats.CombatReflexes,
            StandardFeats.Dodge,
            StandardFeats.Mobility,
            StandardFeats.SpringAttack,
            StandardFeats.ExoticWeaponProficiency,
            StandardFeats.ImprovedCritical,
            StandardFeats.ImprovedInitiative,
            StandardFeats.ImprovedShieldBash,
            StandardFeats.ImprovedUnarmedStrike,
            StandardFeats.DeflectArrows,
            StandardFeats.ImprovedGrapple,
            StandardFeats.SnatchArrows,
            StandardFeats.StunningFist,
            StandardFeats.PointBlankShot,
            StandardFeats.FarShot,
            StandardFeats.PreciseShot,
            StandardFeats.RapidShot,
            StandardFeats.Manyshot,
            StandardFeats.ShotOnTheRun,
            StandardFeats.ImprovedPreciseShot,
            StandardFeats.PowerAttack,
            StandardFeats.Cleave,
            StandardFeats.GreatCleave,
            StandardFeats.ImprovedBullRush,
            StandardFeats.ImprovedOverrun,
            StandardFeats.ImprovedSunder,
            StandardFeats.QuickDraw,
            StandardFeats.RapidReload,
            StandardFeats.TwoWeaponFighting,
            StandardFeats.TwoWeaponDefense,
            StandardFeats.ImprovedTwoWeaponFighting,
            StandardFeats.GreaterTwoWeaponFighting,
            StandardFeats.WeaponFinesse,
            StandardFeats.WeaponFocus,
            StandardFeats.WeaponSpecialization,
            StandardFeats.GreaterWeaponFocus,
            StandardFeats.GreaterWeaponSpecialization
        ].forEach(function (feat) {
            FighterBonusFeats.push(feat);
        });

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
            willSave: SavingThrows.Weak,
            classSkills: [
                StandardSkills.Intimidate,
                StandardSkills.Listen,
                StandardSkills.Survival
            ]
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
            willSave: SavingThrows.Strong,
            classSkills: [
                StandardSkills.Appraise,
                StandardSkills.Bluff,
                StandardSkills.Concentration,
                StandardSkills.Diplomacy,
                StandardSkills.GatherInformation,
                StandardSkills.Hide,
                StandardSkills.Listen,
                StandardSkills.MoveSilently,
                StandardSkills.Perform,
                StandardSkills.SenseMotive,
                StandardSkills.SleightOfHand,
                StandardSkills.Spellcraft,
                StandardSkills.Tumble,
                StandardSkills.UseMagicDevice
            ]
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
            willSave: SavingThrows.Strong,
            classSkills: [
                StandardSkills.Concentration,
                StandardSkills.Diplomacy,
                StandardSkills.Heal,
                StandardSkills.Spellcraft
            ],
            getClassSkills: function(character) {
                // Domains add extra class-skills (possibly)
                var result = this.classSkills.slice(0);
                character.domains.forEach(function (domain) {
                    if (domain.classSkills) {
                        domain.classSkills.forEach(function (classSkill) {
                            if (result.indexOf(classSkill) == -1)
                                result.push(classSkill);
                        });
                    }
                });
                return result;
            }
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
            willSave: SavingThrows.Strong,
            classSkills: [
                StandardSkills.Concentration,
                StandardSkills.Diplomacy,
                StandardSkills.Heal,
                StandardSkills.Listen,
                StandardSkills.Spellcraft,
                StandardSkills.Spot,
                StandardSkills.Survival
            ]
        });

        var fighterBonusFeatEntry = {
            count: 1,
            feats: FighterBonusFeats
        };

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
            willSave: SavingThrows.Weak,
            bonusFeats: {
                1: fighterBonusFeatEntry,
                2: fighterBonusFeatEntry,
                4: fighterBonusFeatEntry,
                6: fighterBonusFeatEntry,
                8: fighterBonusFeatEntry,
                10: fighterBonusFeatEntry,
                12: fighterBonusFeatEntry,
                14: fighterBonusFeatEntry,
                16: fighterBonusFeatEntry,
                18: fighterBonusFeatEntry,
                20: fighterBonusFeatEntry
            },
            classSkills: [
                StandardSkills.Intimidate
            ]
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
            willSave: SavingThrows.Strong,
            classSkills: [
                StandardSkills.Concentration,
                StandardSkills.Diplomacy,
                StandardSkills.Hide,
                StandardSkills.Listen,
                StandardSkills.MoveSilently,
                StandardSkills.Perform,
                StandardSkills.SenseMotive,
                StandardSkills.Spot,
                StandardSkills.Tumble
            ]
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
            willSave: SavingThrows.Weak,
            classSkills: [
                StandardSkills.Concentration,
                StandardSkills.Diplomacy,
                StandardSkills.Heal,
                StandardSkills.SenseMotive
            ]
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
            willSave: SavingThrows.Weak,
            classSkills: [
                StandardSkills.Concentration,
                StandardSkills.Heal,
                StandardSkills.Hide,
                StandardSkills.Listen,
                StandardSkills.MoveSilently,
                StandardSkills.Search,
                StandardSkills.Spot,
                StandardSkills.Survival
            ]
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
            willSave: SavingThrows.Weak,
            classSkills: [
                StandardSkills.Appraise,
                StandardSkills.Bluff,
                StandardSkills.Diplomacy,
                StandardSkills.DisableDevice,
                StandardSkills.GatherInformation,
                StandardSkills.Hide,
                StandardSkills.Intimidate,
                StandardSkills.Listen,
                StandardSkills.MoveSilently,
                StandardSkills.OpenLock,
                StandardSkills.Perform,
                StandardSkills.Search,
                StandardSkills.SenseMotive,
                StandardSkills.SleightOfHand,
                StandardSkills.Spot,
                StandardSkills.Tumble,
                StandardSkills.UseMagicDevice
            ]
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
            willSave: SavingThrows.Strong,
            classSkills: [
                StandardSkills.Bluff,
                StandardSkills.Concentration,
                StandardSkills.Spellcraft
            ]
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
            willSave: SavingThrows.Strong,
            classSkills: [
                StandardSkills.Concentration,
                StandardSkills.Spellcraft
            ]
        });
    }

    StartupListeners.add(registerClasses, 'd20-classes', []);

})();
