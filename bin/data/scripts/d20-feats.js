/**
 * Constants for feat identifiers from the D20 SRD.
 */
var StandardFeats = {
    Acrobatic: 'acrobatic',
    Agile: 'agile',
    Alertness: 'alertness',
    AnimalAffinity: 'animal-affinity',
    ArmorProficiencyLight: 'armor-proficiency-light',
    ArmorProficiencyMedium: 'armor-proficiency-medium',
    ArmorProficiencyHeavy: 'armor-proficiency-heavy',
    Athletic: 'athletic',
    AugmentSummoning: 'augment Summoning',
    BlindFight: 'blind-fight',
    BrewPotion: 'brew-potion',
    Cleave: 'cleave',
    CombatCasting: 'combat-casting',
    CombatExpertise: 'combat-expertise',
    CombatReflexes: 'combat-reflexes',
    CraftMagicArmsAndArmor: 'craft-magic-arms-and-armor',
    CraftRod: 'craft-rod',
    CraftStaff: 'craft-staff',
    CraftWand: 'craft-wand',
    CraftWondrousItem: 'craft-wondrous-item',
    Deceitful: 'deceitful',
    DeftHands: 'deft-hands',
    Diehard: 'diehard',
    Diligent: 'diligent',
    DeflectArrows: 'deflect-arrows',
    Dodge: 'dodge',
    EmpowerSpell: 'empower-spell',
    Endurance: 'endurance',
    EnlargeSpell: 'enlarge-spell',
    EschewMaterials: 'eschew-materials',
    ExoticWeaponProficiency: 'exotic-weapon-proficiency',
    ExtendSpell: 'extend-spell',
    ExtraTurning: 'extra-turning',
    FarShot: 'far-shot',
    ForgeRing: 'forge-ring',
    GreatCleave: 'great-cleave',
    GreatFortitude: 'great-fortitude',
    GreaterSpellFocus: 'greater-spell-focus',
    GreaterSpellPenetration: 'greater-spell-penetration',
    GreaterTwoWeaponFighting: 'greater-two-weapon-fighting',
    GreaterWeaponFocus: 'greater-weapon-focus',
    GreaterWeaponSpecialization: 'greater-weapon-specialization',
    HeightenSpell: 'heighten-spell',
    ImprovedBullRush: 'improved-bull-rush',
    ImprovedCounterspell: 'improved-counterspell',
    ImprovedCritical: 'improved-critical',
    ImprovedDisarm: 'improved-disarm',
    ImprovedFeint: 'improved-feint',
    ImprovedGrapple: 'improved-grapple',
    ImprovedInitiative: 'improved-initiative',
    ImprovedOverrun: 'improved-overrun',
    ImprovedPreciseShot: 'improved-precise-shot',
    ImprovedShieldBash: 'improved-shield-bash',
    ImprovedSunder: 'improved-sunder',
    ImprovedTrip: 'improved-trip',
    ImprovedTwoWeaponFighting: 'improved-two-weapon-fighting',
    ImprovedTurning: 'improved-turning',
    ImprovedUnarmedStrike: 'improved-unarmed-strike',
    ImprovedUncannyDodge: 'improved-uncanny-dodge',
    Investigator: 'investigator',
    IronWill: 'iron-will',
    Leadership: 'leadership',
    LightningReflexes: 'lightning-reflexes',
    MagicalAffinity: 'magical-affinity',
    Manyshot: 'manyshot',
    MartialWeaponProficiency: 'martial-weapon-proficiency',
    MaximizeSpell: 'maximize-spell',
    Mobility: 'mobility',
    NaturalSpell: 'natural-spell',
    Negotiator: 'negotiator',
    NimbleFingers: 'nimble-fingers',
    Persuasive: 'persuasive',
    PointBlankShot: 'point-blank-shot',
    PowerAttack: 'power-attack',
    PreciseShot: 'recise-shot',
    QuickDraw: 'quick-draw',
    QuickenSpell: 'quicken-spell',
    RapidShot: 'rapid-shot',
    RapidReload: 'rapid-reload',
    Run: 'run',
    ScribeScroll: 'scribe-scroll',
    SelfSufficient: 'self-sufficient',
    ShieldProficiency: 'shield-proficiency',
    ShotOnTheRun: 'shot-on-the-run',
    SilentSpell: 'silent-spell',
    SimpleWeaponProficiency: 'simple-weapon-proficiency',
    SkillFocus: 'skill-focus',
    SnatchArrows: 'Snatch Arrows',
    SpellFocus: 'spell-focus-abjuration',
    SpellMastery: 'spell-mastery',
    SpellPenetration: 'spell-penetration',
    SpringAttack: 'spring-attack',
    Stealthy: 'stealthy',
    StillSpell: 'still-spell',
    StunningFist: 'stunning-fist',
    Toughness: 'toughness',
    TowerShieldProficiency: 'tower-shield-proficiency',
    Track: 'track',
    TwoWeaponFighting: 'two-weapon-fighting',
    TwoWeaponDefense: 'two-weapon-defense',
    WeaponFinesse: 'weapon-finesse',
    WeaponFocus: 'weapon-focus',
    WeaponSpecialization: 'weapon-specialization',
    WhirlwindAttack: 'whirlwind-attack',
    WidenSpell: 'widen-spell'
};

var StandardFeatCategories = {
    General: 'general',
    ItemCreation: 'itemcreation',
    Metamagic: 'metamagic'
};

(function() {

    /**
     * Strips any extra qualifiers that are in parenthesis at the end of the name from a string.
     * @param name The string.
     */
    function stripNameExtra(name) {
        var idx = name.indexOf('(');
        if (idx != -1) {
            name = name.substr(0, idx);
            // Strip the now trailing whitespace
            while (name[name.length - 1] == ' ')
                name = name.substr(0, name.length - 1);
        }
        return name;
    }

    function registerFeats() {

        Feats.register({
            id: StandardFeats.Acrobatic,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/0'),
            shortDescription: translations.get('mes/feat/5000')
        });

        Feats.register({
            id: StandardFeats.Agile,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/1'),
            shortDescription: translations.get('mes/feat/5001')
        });

        Feats.register({
            id: StandardFeats.Alertness,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/2'),
            shortDescription: translations.get('mes/feat/5002')
        });

        Feats.register({
            id: StandardFeats.AnimalAffinity,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/3'),
            shortDescription: translations.get('mes/feat/5003')
        });

        Feats.register({
            id: StandardFeats.ArmorProficiencyLight,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/4'),
            shortDescription: translations.get('mes/feat/5004')
        });

        Feats.register({
            id: StandardFeats.ArmorProficiencyMedium,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/5'),
            shortDescription: translations.get('mes/feat/5005'),
            requirements: [
                FeatRequirement(StandardFeats.ArmorProficiencyLight)
            ]
        });

        Feats.register({
            id: StandardFeats.ArmorProficiencyHeavy,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/6'),
            shortDescription: translations.get('mes/feat/5006'),
            requirements: [
                FeatRequirement(StandardFeats.ArmorProficiencyMedium)
            ]
        });

        Feats.register({
            id: StandardFeats.Athletic,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/7'),
            shortDescription: translations.get('mes/feat/5007')
        });

        Feats.register({
            id: StandardFeats.AugmentSummoning,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/8'),
            shortDescription: translations.get('mes/feat/5008'),
            requirements: [
                FeatRequirement(StandardFeats.SpellFocus, StandardSchools.Conjuration)
            ]
        });

        Feats.register({
            id: StandardFeats.BlindFight,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/9'),
            shortDescription: translations.get('mes/feat/5009')
        });

        Feats.register({
            id: StandardFeats.BrewPotion,
            category: StandardFeatCategories.ItemCreation,
            name: translations.get('mes/feat/10'),
            shortDescription: translations.get('mes/feat/5010'),
            requirements: [
                CasterLevelRequirement(CasterLevelRequirement.Any, 3)
            ]
        });

        Feats.register({
            id: StandardFeats.Cleave,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/11'),
            shortDescription: translations.get('mes/feat/5011'),
            requirements: [
                AbilityRequirement(Abilities.Strength, 13),
                FeatRequirement(StandardFeats.PowerAttack)
            ]
        });

        Feats.register({
            id: StandardFeats.CombatCasting,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/12'),
            shortDescription: translations.get('mes/feat/5012')
        });

        Feats.register({
            id: StandardFeats.CombatExpertise,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/13'),
            shortDescription: translations.get('mes/feat/5013'),
            requirements: [
                AbilityRequirement(Abilities.Intelligence, 13)
            ]
        });

        Feats.register({
            id: StandardFeats.CombatReflexes,
            category: StandardFeatCategories.General,
            name: qsTr('Combat Reflexes'),
            shortDescription: qsTr('You may make additional attacks of opportunity per round and while you are flat-footed.')
        });

        Feats.register({
            id: StandardFeats.CraftMagicArmsAndArmor,
            category: StandardFeatCategories.ItemCreation,
            name: translations.get('mes/feat/14'),
            shortDescription: translations.get('mes/feat/5014'),
            requirements: [
                CasterLevelRequirement(CasterLevelRequirement.Any, 5)
            ]
        });

        Feats.register({
            id: StandardFeats.CraftRod,
            category: StandardFeatCategories.ItemCreation,
            name: translations.get('mes/feat/15'),
            shortDescription: translations.get('mes/feat/5015'),
            requirements: [
                CasterLevelRequirement(CasterLevelRequirement.Any, 9)
            ]
        });

        Feats.register({
            id: StandardFeats.CraftStaff,
            category: StandardFeatCategories.ItemCreation,
            name: translations.get('mes/feat/16'),
            shortDescription: translations.get('mes/feat/5016'),
            requirements: [
                CasterLevelRequirement(CasterLevelRequirement.Any, 12)
            ]
        });

        Feats.register({
            id: StandardFeats.CraftWand,
            category: StandardFeatCategories.ItemCreation,
            name: translations.get('mes/feat/17'),
            shortDescription: translations.get('mes/feat/5017'),
            requirements: [
                CasterLevelRequirement(CasterLevelRequirement.Any, 5)
            ]
        });

        Feats.register({
            id: StandardFeats.CraftWondrousItem,
            category: StandardFeatCategories.ItemCreation,
            name: translations.get('mes/feat/18'),
            shortDescription: translations.get('mes/feat/5018'),
            requirements: [
                CasterLevelRequirement(CasterLevelRequirement.Any, 3)
            ]
        });

        Feats.register({
            id: StandardFeats.Deceitful,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/19'),
            shortDescription: translations.get('mes/feat/5019')
        });

        Feats.register({
            id: StandardFeats.DeflectArrows,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/23'),
            shortDescription: translations.get('mes/feat/5023'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 13),
                FeatRequirement(StandardFeats.ImprovedUnarmedStrike)
            ]
        });

        Feats.register({
            id: StandardFeats.DeftHands,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/20'),
            shortDescription: translations.get('mes/feat/5020')
        });

        Feats.register({
            id: StandardFeats.Diehard,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/21'),
            shortDescription: translations.get('mes/feat/5021'),
            requirements: [
                FeatRequirement(StandardFeats.Endurance)
            ]
        });

        Feats.register({
            id: StandardFeats.Diligent,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/22'),
            shortDescription: translations.get('mes/feat/5022')
        });

        Feats.register({
            id: StandardFeats.Dodge,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/24'),
            shortDescription: translations.get('mes/feat/5024'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 13)
            ]
        });

        Feats.register({
            id: StandardFeats.EmpowerSpell,
            category: StandardFeatCategories.Metamagic,
            name: translations.get('mes/feat/25'),
            shortDescription: translations.get('mes/feat/5025')
        });

        Feats.register({
            id: StandardFeats.Endurance,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/26'),
            shortDescription: translations.get('mes/feat/5026')
        });

        Feats.register({
            id: StandardFeats.EnlargeSpell,
            category: StandardFeatCategories.Metamagic,
            name: translations.get('mes/feat/27'),
            shortDescription: translations.get('mes/feat/5027')
        });

        Feats.register({
            id: StandardFeats.EschewMaterials,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/28'),
            shortDescription: translations.get('mes/feat/5028')
        });

        // Build a list of all exotic weapons for the selection box.
        var exoticWeaponIds = Weapons.getByCategory(StandardWeaponCategories.Exotic).map(function (weapon) {
            return {
                id: weapon.id,
                text: weapon.name
            }
        });

        Feats.register({
            id: StandardFeats.ExoticWeaponProficiency,
            category: StandardFeatCategories.General,
            name: translations.get('mes/pc_creation/19101'),
            shortDescription: translations.get('mes/feat/5029'),
            requirements: [
                BaseAttackBonusRequirement(1),
                ConditionalRequirement(StandardWeapons.BastardSword, AbilityRequirement(Abilities.Strength, 13)),
                ConditionalRequirement(StandardWeapons.DwarvenWaraxe, AbilityRequirement(Abilities.Strength, 13))
            ],
            argument: FeatArgument(qsTr('Weapon Type'),
                    qsTr('You will gain proficiency with the weapon type you choose.'),
                    exoticWeaponIds)
        });

        Feats.register({
            id: StandardFeats.ExtendSpell,
            category: StandardFeatCategories.Metamagic,
            name: translations.get('mes/feat/49'),
            shortDescription: translations.get('mes/feat/5049')
        });

        Feats.register({
            id: StandardFeats.ExtraTurning,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/50'),
            shortDescription: translations.get('mes/feat/5050'),
            requirements: [
                TurnOrRebukeRequirement()
            ]
        });

        Feats.register({
            id: StandardFeats.FarShot,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/51'),
            shortDescription: translations.get('mes/feat/5051'),
            requirements: [
                FeatRequirement(StandardFeats.PointBlankShot)
            ]
        });

        Feats.register({
            id: StandardFeats.ForgeRing,
            category: StandardFeatCategories.ItemCreation,
            name: translations.get('mes/feat/52'),
            shortDescription: translations.get('mes/feat/5052'),
            requirements: [
                CasterLevelRequirement(CasterLevelRequirement.Any, 12)
            ]
        });

        Feats.register({
            id: StandardFeats.GreatCleave,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/53'),
            shortDescription: translations.get('mes/feat/5053'),
            requirements: [
                AbilityRequirement(Abilities.Strength, 13),
                FeatRequirement(StandardFeats.Cleave),
                FeatRequirement(StandardFeats.PowerAttack),
                BaseAttackBonusRequirement(4)
            ]
        });

        Feats.register({
            id: StandardFeats.GreatFortitude,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/54'),
            shortDescription: translations.get('mes/feat/5054')
        });

        Feats.register({
            id: StandardFeats.GreaterSpellFocus,
            category: StandardFeatCategories.Metamagic,
            name: stripNameExtra(translations.get('mes/feat/55')),
            shortDescription: translations.get('mes/feat/5055'),
            requirements: [
                FeatRequirement(StandardFeats.SpellFocus, FeatRequirement.SameArgument)
            ]
        });

        Feats.register({
            id: StandardFeats.GreaterSpellPenetration,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/63'),
            shortDescription: translations.get('mes/feat/5063'),
            requirements: [
                FeatRequirement(StandardFeats.SpellPenetration)
            ]
        });

        Feats.register({
            id: StandardFeats.GreaterTwoWeaponFighting,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/64'),
            shortDescription: translations.get('mes/feat/5064'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 19),
                FeatRequirement(StandardFeats.ImprovedTwoWeaponFighting),
                FeatRequirement(StandardFeats.TwoWeaponFighting),
                BaseAttackBonusRequirement(11)
            ]
        });

        Feats.register({
            id: StandardFeats.GreaterWeaponFocus,
            category: StandardFeatCategories.General,
            name: translations.get('mes/pc_creation/19108'),
            shortDescription: translations.get('mes/feat/5065'),
            requirements: [
                FeatRequirement(StandardFeats.WeaponFocus, FeatRequirement.SameArgument),
                ClassLevelRequirement(StandardClasses.Fighter, 8)
            ]
        });

        Feats.register({
            id: StandardFeats.GreaterWeaponSpecialization,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/137'),
            shortDescription: translations.get('mes/feat/5137'),
            requirements: [
                FeatRequirement(StandardFeats.GreaterWeaponFocus, FeatRequirement.SameArgument),
                FeatRequirement(StandardFeats.WeaponSpecialization, FeatRequirement.SameArgument),
                ClassLevelRequirement(StandardClasses.Fighter, 12)
            ]
        });

        Feats.register({
            id: StandardFeats.HeightenSpell,
            category: StandardFeatCategories.Metamagic,
            name: translations.get('mes/feat/138'),
            shortDescription: translations.get('mes/feat/5138')
        });

        Feats.register({
            id: StandardFeats.ImprovedBullRush,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/139'),
            shortDescription: translations.get('mes/feat/5139'),
            requirements: [
                AbilityRequirement(Abilities.Strength, 13),
                FeatRequirement(StandardFeats.PowerAttack)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedCounterspell,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/140'),
            shortDescription: translations.get('mes/feat/5140')
        });

        Feats.register({
            id: StandardFeats.ImprovedCritical,
            category: StandardFeatCategories.General,
            name: stripNameExtra(translations.get('mes/feat/141')),
            shortDescription: translations.get('mes/feat/5141'),
            requirements: [
                ProficientWithWeaponRequirement(),
                BaseAttackBonusRequirement(8)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedDisarm,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/211'),
            shortDescription: translations.get('mes/feat/5211'),
            requirements: [
                AbilityRequirement(Abilities.Intelligence, 13),
                FeatRequirement(StandardFeats.CombatExpertise)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedFeint,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/212'),
            shortDescription: translations.get('mes/feat/5212'),
            requirements: [
                AbilityRequirement(Abilities.Intelligence, 13),
                FeatRequirement(StandardFeats.CombatExpertise)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedGrapple,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/213'),
            shortDescription: translations.get('mes/feat/5213'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 13),
                FeatRequirement(StandardFeats.ImprovedUnarmedStrike)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedInitiative,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/214'),
            shortDescription: translations.get('mes/feat/5214')
        });

        Feats.register({
            id: StandardFeats.ImprovedOverrun,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/215'),
            shortDescription: translations.get('mes/feat/5215'),
            requirements: [
                AbilityRequirement(Abilities.Strength, 13),
                FeatRequirement(StandardFeats.PowerAttack)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedPreciseShot,
            category: StandardFeatCategories.General,
            name: qsTr('Improved Precise Shot'),
            shortDescription: qsTr('Your ranged attacks ignore the AC bonus granted to targets by anything less than total cover, and the miss chance granted to targets by anything less than total concealment.'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 19),
                FeatRequirement(StandardFeats.PreciseShot),
                BaseAttackBonusRequirement(11)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedShieldBash,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/216'),
            shortDescription: translations.get('mes/feat/5216'),
            requirements: [
                FeatRequirement(StandardFeats.ShieldProficiency)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedSunder,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/340'),
            shortDescription: translations.get('mes/feat/5340'),
            requirements: [
                FeatRequirement(StandardFeats.PowerAttack)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedTrip,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/217'),
            shortDescription: translations.get('mes/feat/5217'),
            requirements: [
                AbilityRequirement(Abilities.Intelligence, 13),
                FeatRequirement(StandardFeats.CombatExpertise)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedTurning,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/219'),
            shortDescription: translations.get('mes/feat/5219'),
            requirements: [
                TurnOrRebukeRequirement()
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedTwoWeaponFighting,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/218'),
            shortDescription: translations.get('mes/feat/5218'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 17),
                FeatRequirement(StandardFeats.TwoWeaponFighting),
                BaseAttackBonusRequirement(6)
            ]
        });

        Feats.register({
            id: StandardFeats.ImprovedUnarmedStrike,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/220'),
            shortDescription: translations.get('mes/feat/5220')
        });

        Feats.register({
            id: StandardFeats.ImprovedUncannyDodge,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/221'),
            shortDescription: translations.get('mes/feat/5221')
        });

        Feats.register({
            id: StandardFeats.Investigator,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/222'),
            shortDescription: translations.get('mes/feat/5222')
        });

        Feats.register({
            id: StandardFeats.IronWill,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/223'),
            shortDescription: translations.get('mes/feat/5223')
        });

        Feats.register({
            id: StandardFeats.Leadership,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/224'),
            shortDescription: translations.get('mes/feat/5224'),
            requirements: [
                CharacterLevelRequirement(6)
            ]
        });

        Feats.register({
            id: StandardFeats.LightningReflexes,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/225'),
            shortDescription: translations.get('mes/feat/5225')
        });

        Feats.register({
            id: StandardFeats.MagicalAffinity,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/226'),
            shortDescription: translations.get('mes/feat/5226')
        });

        Feats.register({
            id: StandardFeats.Manyshot,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/227'),
            shortDescription: translations.get('mes/feat/5227'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 17),
                FeatRequirement(StandardFeats.PointBlankShot),
                FeatRequirement(StandardFeats.RapidShot),
                BaseAttackBonusRequirement(6)
            ]
        });

        Feats.register({
            id: StandardFeats.MartialWeaponProficiency,
            category: StandardFeatCategories.General,
            name: stripNameExtra(translations.get('mes/feat/228')),
            shortDescription: translations.get('mes/feat/5228')
        });

        Feats.register({
            id: StandardFeats.MaximizeSpell,
            category: StandardFeatCategories.Metamagic,
            name: translations.get('mes/feat/259'),
            shortDescription: translations.get('mes/feat/5259')
        });

        Feats.register({
            id: StandardFeats.Mobility,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/260'),
            shortDescription: translations.get('mes/feat/5260'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 13),
                FeatRequirement(StandardFeats.Dodge)
            ]
        });

        Feats.register({
            id: StandardFeats.NaturalSpell,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/263'),
            shortDescription: translations.get('mes/feat/5263'),
            requirements: [
                AbilityRequirement(Abilities.Wisdom, 13),
                WildShapeAbilityRequirement()
            ]
        });

        Feats.register({
            id: StandardFeats.Negotiator,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/264'),
            shortDescription: translations.get('mes/feat/5264')
        });

        Feats.register({
            id: StandardFeats.NimbleFingers,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/265'),
            shortDescription: translations.get('mes/feat/5265')
        });

        Feats.register({
            id: StandardFeats.Persuasive,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/266'),
            shortDescription: translations.get('mes/feat/5266')
        });

        Feats.register({
            id: StandardFeats.PointBlankShot,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/267'),
            shortDescription: translations.get('mes/feat/5267')
        });

        Feats.register({
            id: StandardFeats.PowerAttack,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/268'),
            shortDescription: translations.get('mes/feat/5268'),
            requirements: [
                AbilityRequirement(Abilities.Strength, 13)
            ]
        });

        Feats.register({
            id: StandardFeats.PreciseShot,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/269'),
            shortDescription: translations.get('mes/feat/5269'),
            requirements: [
                FeatRequirement(StandardFeats.PointBlankShot)
            ]
        });

        Feats.register({
            id: StandardFeats.QuickDraw,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/270'),
            shortDescription: translations.get('mes/feat/5270'),
            requirements: [
                BaseAttackBonusRequirement(1)
            ]
        });

        Feats.register({
            id: StandardFeats.QuickenSpell,
            category: StandardFeatCategories.Metamagic,
            name: translations.get('mes/feat/271'),
            shortDescription: translations.get('mes/feat/5271')
        });

        Feats.register({
            id: StandardFeats.RapidReload,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/273'),
            shortDescription: translations.get('mes/feat/5273'),
            requirements: [
                ProficientWithWeaponRequirement()
            ]
        });

        Feats.register({
            id: StandardFeats.RapidShot,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/272'),
            shortDescription: translations.get('mes/feat/5272'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 13),
                FeatRequirement(StandardFeats.PointBlankShot)
            ]
        });

        Feats.register({
            id: StandardFeats.Run,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/275'),
            shortDescription: translations.get('mes/feat/5275')
        });

        Feats.register({
            id: StandardFeats.ScribeScroll,
            category: StandardFeatCategories.ItemCreation,
            name: translations.get('mes/feat/276'),
            shortDescription: translations.get('mes/feat/5276'),
            requirements: [
                CasterLevelRequirement(CasterLevelRequirement.Any, 1)
            ]
        });

        Feats.register({
            id: StandardFeats.SelfSufficient,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/277'),
            shortDescription: translations.get('mes/feat/5277')
        });

        Feats.register({
            id: StandardFeats.ShieldProficiency,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/278'),
            shortDescription: translations.get('mes/feat/5278')
        });

        Feats.register({
            id: StandardFeats.ShotOnTheRun,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/279'),
            shortDescription: translations.get('mes/feat/5279'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 13),
                FeatRequirement(StandardFeats.Dodge),
                FeatRequirement(StandardFeats.Mobility),
                FeatRequirement(StandardFeats.PointBlankShot),
                BaseAttackBonusRequirement(4)
            ]
        });

        Feats.register({
            id: StandardFeats.SilentSpell,
            category: StandardFeatCategories.Metamagic,
            name: translations.get('mes/feat/280'),
            shortDescription: translations.get('mes/feat/5280')
        });

        Feats.register({
            id: StandardFeats.SimpleWeaponProficiency,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/281'),
            shortDescription: translations.get('mes/feat/5281')
        });

        Feats.register({
            id: StandardFeats.SkillFocus,
            category: StandardFeatCategories.General,
            name: stripNameExtra(translations.get('mes/feat/282')),
            shortDescription: translations.get('mes/feat/5282')
        });

        Feats.register({
            id: StandardFeats.SnatchArrows,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/324'),
            shortDescription: translations.get('mes/feat/5324'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 15),
                FeatRequirement(StandardFeats.DeflectArrows),
                FeatRequirement(StandardFeats.ImprovedUnarmedStrike)
            ]
        });

        Feats.register({
            id: StandardFeats.SpellFocus,
            category: StandardFeatCategories.General,
            name: stripNameExtra(translations.get('mes/feat/325')),
            shortDescription: translations.get('mes/feat/5325')
        });

        Feats.register({
            id: StandardFeats.SpellMastery,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/333'),
            shortDescription: translations.get('mes/feat/5333'),
            requirements: [
                ClassLevelRequirement(StandardClasses.Wizard, 1)
            ]
        });

        Feats.register({
            id: StandardFeats.SpellPenetration,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/334'),
            shortDescription: translations.get('mes/feat/5334')
        });

        Feats.register({
            id: StandardFeats.SpringAttack,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/336'),
            shortDescription: translations.get('mes/feat/5336'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 13),
                FeatRequirement(StandardFeats.Dodge),
                FeatRequirement(StandardFeats.Mobility),
                BaseAttackBonusRequirement(4)
            ]
        });

        Feats.register({
            id: StandardFeats.Stealthy,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/337'),
            shortDescription: translations.get('mes/feat/5337')
        });

        Feats.register({
            id: StandardFeats.StillSpell,
            category: StandardFeatCategories.Metamagic,
            name: translations.get('mes/feat/338'),
            shortDescription: translations.get('mes/feat/5338')
        });

        Feats.register({
            id: StandardFeats.StunningFist,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/339'),
            shortDescription: translations.get('mes/feat/5339'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 13),
                AbilityRequirement(Abilities.Wisdom, 13),
                FeatRequirement(StandardFeats.ImprovedUnarmedStrike),
                BaseAttackBonusRequirement(8)
            ]
        });

        Feats.register({
            id: StandardFeats.Toughness,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/341'),
            shortDescription: translations.get('mes/feat/5341')
        });

        Feats.register({
            id: StandardFeats.TowerShieldProficiency,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/342'),
            shortDescription: translations.get('mes/feat/5342'),
            requirements: [
                FeatRequirement(StandardFeats.ShieldProficiency)
            ]
        });

        Feats.register({
            id: StandardFeats.Track,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/343'),
            shortDescription: translations.get('mes/feat/5343')
        });

        Feats.register({
            id: StandardFeats.TwoWeaponFighting,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/345'),
            shortDescription: translations.get('mes/feat/5345'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 15)
            ]
        });

        Feats.register({
            id: StandardFeats.TwoWeaponDefense,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/346'),
            shortDescription: translations.get('mes/feat/5346'),
            requirements: [
                FeatRequirement(StandardFeats.TwoWeaponFighting)
            ]
        });

        Feats.register({
            id: StandardFeats.WeaponFinesse,
            category: StandardFeatCategories.General,
            name: translations.get('mes/pc_creation/19105'),
            shortDescription: translations.get('mes/feat/5347'),
            requirements: [
                BaseAttackBonusRequirement(1)
            ]
        });

        Feats.register({
            id: StandardFeats.WeaponFocus,
            category: StandardFeatCategories.General,
            name: translations.get('mes/pc_creation/19106'),
            shortDescription: translations.get('mes/feat/5417'),
            requirements: [
                BaseAttackBonusRequirement(1),
                ProficientWithWeaponRequirement()
            ]
        });

        Feats.register({
            id: StandardFeats.WeaponSpecialization,
            category: StandardFeatCategories.General,
            name: translations.get('mes/pc_creation/19107'),
            shortDescription: translations.get('mes/feat/5489'),
            requirements: [
                FeatRequirement(StandardFeats.WeaponFocus, FeatRequirement.SameArgument),
                ClassLevelRequirement(StandardClasses.Fighter, 4)
            ]
        });

        Feats.register({
            id: StandardFeats.WhirlwindAttack,
            category: StandardFeatCategories.General,
            name: translations.get('mes/feat/560'),
            shortDescription: translations.get('mes/feat/5560'),
            requirements: [
                AbilityRequirement(Abilities.Dexterity, 13),
                AbilityRequirement(Abilities.Intelligence, 13),
                FeatRequirement(StandardFeats.CombatExpertise),
                FeatRequirement(StandardFeats.Dodge),
                FeatRequirement(StandardFeats.Mobility),
                FeatRequirement(StandardFeats.SpringAttack),
                BaseAttackBonusRequirement(4)
            ]
        });

        Feats.register({
            id: StandardFeats.WidenSpell,
            category: StandardFeatCategories.Metamagic,
            name: translations.get('mes/feat/645'),
            shortDescription: translations.get('mes/feat/5645')
        });
    }

    StartupListeners.add(registerFeats, 'd20-feats', ['d20-weapons']);

})();
