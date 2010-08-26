
var PixelPerWorldTile = 28.2842703;

// Maps legacy map id to map name (e.g. 5001 -> Map-2-Hommlet-Exterior)
var LegacyMapList = {};

var npcAddMeshes = {}; // Maps npc addmesh ids to the actual addmeshes.

/**
   Called for stairs-down, stairs-up and other teleport icons.
   ALL have [ "SeeThrough", "ShootThrough", "NoBlock", "ProvidesCover", "Invulnerable" ]
  */
function teleportIcon(proto) {
        proto.type = 'MapChanger';
        proto.flags = ["SeeThrough", "ShootThrough", "NoBlock", "ProvidesCover", "Invulnerable"];
}

function money(proto) {
    proto.flags = ["Flat", "SeeThrough", "ShootThrough", "NoBlock"];
}

var processors = {};
processors.register = function(func, ids) {
        for (var i = 0; i < ids.length; ++i)
                this[ids[i]] = func;
};

processors.register(money, ['7000', '7001', '7002', '7003']);
processors.register(teleportIcon, ['2011', '2012', '2013', '2014', '2015', '2035', '2036', '2037', '2038', '2039']);

var hairTypes = {
        "Longhair (m/f)": 0,
        "Ponytail (m/f)": 1,
        "Shorthair (m/f)": 2,
        "Topknot (m/f)": 3,
        "Mullet (m)": 4,
        "Pigtails (f)": 5,
        "Bald (m)": 6,
        "Braids (f)": 7,
        "Mohawk (m/f)": 8,
        "Medium (m)": 9,
        "Ponytail2 (f)": 10
};

var hairColors = {
        "Black": 0,
        "Blonde": 1,
        "Blue": 2,
        "Brown": 3,
        "Light Brown": 4,
        "Pink": 5,
        "Red": 6,
        "White": 7
};

var AlignmentMap = {
    'align_lawful_good': 'lawful_good',
    'align_neutral_good': 'neutral_good',
    'align_chaotic_good': 'chaotic_good',

    'align_lawful_neutral': 'lawful_neutral',
    'align_true_neutral': 'true_neutral',
    'align_chaotic_neutral': 'chaotic_neutral',

    'align_lawful_evil': 'lawful_evil',
    'align_neutral_evil': 'neutral_evil',
    'align_chaotic_evil': 'chaotic_evil'
};

// Maps ToEE deity names to new deity identifiers
var DeityMap = {
    'Boccob': 'boccob',
    'Corellon Larethian': 'corellonlarethian',
    'Ehlonna': 'ehlonna',
    'Erythnul': 'erythnul',
    'Fharlanghn': 'fharlanghn',
    'Garl Glittergold': 'garlglittergold',
    'Gruumsh': 'gruumsh',
    'Heironeous': 'heironeous',
    'Hextor': 'hextor',
    'Kord': 'kord',
    'Moradin': 'moradin',
    'Nerull': 'nerull',
    'Obad-Hai': 'obadhai',
    'Olidammara': 'olidammara',
    'Pelor': 'pelor',
    'St. Cuthbert': 'stcuthbert',
    'Vecna': 'vecna',
    'Wee Jas': 'weejas',
    'Yondalla': 'yondalla',
    'Old Faith': 'oldfaith',
    'Zuggtmoy': 'zuggtmoy',
    'Iuz': 'iuz',
    'Lolth': 'lolth',
    'Procan': 'procan',
    'Norebo': 'norebo',
    'Pyremius': 'pyremius',
    'Ralishaz': 'ralishaz'
};

// Keep these in sync with the game scripts

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

var StandardSchools = {
    Abjuration: 'abjuration',
    Conjuration: 'conjuration',
    Divination: 'divination',
    Enchantment: 'enchantment',
    Evocation: 'evocation',
    Illusion: 'illusion',
    Necromancy: 'necromancy',
    Transmutation: 'transmutation'
};

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

var StandardWeapons = {
    Ray: 'ray',
    Grapple: 'grapple',

    Gauntlet: 'gauntlet',
    UnarmedStrike: 'unarmed-strike',
    Dagger: 'dagger',
    PunchingDagger: 'punching-dagger',
    SpikedGauntlet: 'spiked-gauntlet',
    LightMace: 'light-mace',
    Sickle: 'light-sickle',
    Club: 'club',
    HeavyMace: 'heavy-mace',
    Morningstar: 'morningstar',
    Shortspear: 'shortspear',
    Longspear: 'longspear',
    Quarterstaff: 'quarterstaff',
    Spear: 'spear',
    HeavyCrossbow: 'heavy-crossbow',
    LightCrossbow: 'light-crossbow',
    Dart: 'dart',
    Javelin: 'javelin',
    Sling: 'sling',

    ThrowingAxe: 'throwing-axe',
    LightHammer: 'light-hammer',
    Handaxe: 'handaxe',
    Kukri: 'kukri',
    LightPick: 'light-pick',
    Sap: 'sap',
    LightShield: 'light-shield',
    SpikedArmor: 'spiked-armor',
    LightSpikedShield: 'light-spiked-shield',
    ShortSword: 'shortsword',
    Battleaxe: 'battleaxe',
    Flail: 'flail',
    Longsword: 'longsword',
    HeavyPick: 'heavypick',
    Rapier: 'rapier',
    Scimitar: 'scimitar',
    HeavyShield: 'heavy-shield',
    HeavySpikedShield: 'heavy-spiked-shield',
    Trident: 'trident',
    Warhammer: 'warhammer',
    Falchion: 'falchion',
    Glaive: 'glaive',
    Greataxe: 'greataxe',
    Greatclub: 'greatclub',
    HeavyFlail: 'heavyflail',
    Greatsword: 'greatsword',
    Guisarme: 'guisarme',
    Halberd: 'halberd',
    Lance: 'lance',
    Ranseur: 'ranseur',
    Scythe: 'scythe',

    Longbow: 'longbow',
    CompositeLongbow: 'composite-longbow',
    Shortbow: 'shortbow',
    CompositeShortbow: 'composite-shortbow',

    Kama: 'kama',
    Nunchaku: 'nunchaku',
    Sai: 'sai',
    Siangham: 'siangham',
    BastardSword: 'bastardsword',
    DwarvenWaraxe: 'dwarven-waraxe',
    Whip: 'whip',
    OrcDoubleAxe: 'orc-double-axe',
    SpikedChain: 'spiked-chain',
    DireFlail: 'dire-flail',
    HookedGnomeHammer: 'hooked-gnome-hammer',
    TwoBladedSword: 'two-bladed-sword',
    DwarvenUrgrosh: 'dwarven-urgrosh',
    Bolas: 'bolas',
    HandCrossbow: 'hand-crossbow',
    HeavyRepeatingCrossbow: 'heavy-repeating-crossbow',
    LightRepeatingCrossbow: 'light-repeating-crossbow',
    Net: 'net',
    Shuriken: 'shuriken'
};


// Maps ToEE skill names to the newer identifiers
var SkillMap = {
    Appraise: 'appraise',
    Bluff: 'bluff',
    Concentration: 'concentration',
    Diplomacy: 'diplomacy',
    'Disable Device': 'disable-device',
    'Gather Information': 'gather-information',
    Heal: 'heal',
    Hide: 'hide',
    Intimidate: 'intimidate',
    Listen: 'listen',
    'Move Silently': 'move-silently',
    "Open Lock": 'open-lock',
    Perform: 'perform',
    Search: 'search',
    "Sense Motive": 'sense-motive',
    "Sleight of Hand": 'sleight-of-hand',
    Spellcraft: 'spellcraft',
    Spot: 'spot',
    Survival: 'survival',
    Tumble: 'tumble',
    "Use Magic Device": 'use-magic-device'
};

// Maps from ToEE feats to new feat instances
var FeatMapping = {
    'Acrobatic': StandardFeats.Acrobatic,
    'Agile': StandardFeats.Agile,
    'Alertness': StandardFeats.Alertness,
    'Animal Affinity': StandardFeats.AnimalAffinity,
    'Armor Proficiency (light)': StandardFeats.ArmorProficiencyLight,
    'Armor Proficiency (medium)': StandardFeats.ArmorProficiencyMedium,
    'Armor Proficiency (heavy)': StandardFeats.ArmorProficiencyHeavy,
    'Athletic': StandardFeats.Athletic,
    'Augment Summoning': StandardFeats.AugmentSummoning,
    'Blind-fight': StandardFeats.BlindFight,
    'Brew Potion': StandardFeats.BrewPotion,
    'Cleave': StandardFeats.Cleave,
    'Combat Casting': StandardFeats.CombatCasting,
    'Combat Expertise': StandardFeats.CombatExpertise,
    'Craft Magic Arms and Armor': StandardFeats.CraftMagicArmsAndArmor,
    'Craft Rod': StandardFeats.CraftRod,
    'Craft Staff': StandardFeats.CraftStaff,
    'Craft Wand': StandardFeats.CraftWand,
    'Craft Wondrous Item': StandardFeats.CraftWondrousItem,
    'Deceitful': StandardFeats.Deceitful,
    'Deft Hands': StandardFeats.DeftHands,
    'Diehard': StandardFeats.Diehard,
    'Diligent': StandardFeats.Diligent,
    'Deflect Arrows': StandardFeats.DeflectArrows,
    'Dodge': StandardFeats.Dodge,
    'Empower Spell': StandardFeats.EmpowerSpell,
    'Endurance': StandardFeats.Endurance,
    'Enlarge Spell': StandardFeats.EnlargeSpell,
    'Eschew Materials': StandardFeats.EschewMaterials,
    'Exotic Weapon Proficiency (Halfling Kama)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.Kama],
    'Exotic Weapon Proficiency (Kukri)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.Kukri],
    'Exotic Weapon Proficiency (Halfling Nunchaku)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.Nunchaku],
    'Exotic Weapon Proficiency (Halfling Siangham)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.Siangham],
    'Exotic Weapon Proficiency (Kama)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.Kama],
    'Exotic Weapon Proficiency (Nunchaku)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.Nunchaku],
    'Exotic Weapon Proficiency (Siangham)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.Siangham],
    'Exotic Weapon Proficiency (Bastard Sword)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.BastardSword],
    'Exotic Weapon Proficiency (Dwarven Waraxe)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.DwarvenWaraxe],
    'Exotic Weapon Proficiency (Gnome Hooked Hammer)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.HookedGnomeHammer],
    'Exotic Weapon Proficiency (Orc Double Axe)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.OrcDoubleAxe],
    'Exotic Weapon Proficiency (Spike Chain)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.SpikedChain],
    'Exotic Weapon Proficiency (Dire Flail)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.DireFlail],
    'Exotic Weapon Proficiency (Two-bladed Sword)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.TwoBladedSword],
    'Exotic Weapon Proficiency (Dwarven Urgrosh)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.DwarvenUrgrosh],
    'Exotic Weapon Proficiency (Hand Crossbow)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.HandCrossbow],
    'Exotic Weapon Proficiency (Shuriken)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.Shuriken],
    'Exotic Weapon Proficiency (Whip)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.Whip],
    'Exotic Weapon Proficiency (Repeating Crossbow)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.LightRepeatingCrossbow],
    'Exotic Weapon Proficiency (Net)': [StandardFeats.ExoticWeaponProficiency, StandardWeapons.Net],
    'Extend Spell': StandardFeats.ExtendSpell,
    'Extra Turning': StandardFeats.ExtraTurning,
    'Far Shot': StandardFeats.FarShot,
    'Forge Ring': StandardFeats.ForgeRing,
    'Great Cleave': StandardFeats.GreatCleave,
    'Great Fortitude': StandardFeats.GreatFortitude,
    'Greater Spell Focus (Abjuration)': [StandardFeats.GreaterSpellFocus, StandardSchools.Abjuration],
    'Greater Spell Focus (Conjuration)': [StandardFeats.GreaterSpellFocus, StandardSchools.Conjuration],
    'Greater Spell Focus (Divination)': [StandardFeats.GreaterSpellFocus, StandardSchools.Divination],
    'Greater Spell Focus (Enchantment)': [StandardFeats.GreaterSpellFocus, StandardSchools.Enchantment],
    'Greater Spell Focus (Evocation)': [StandardFeats.GreaterSpellFocus, StandardSchools.Evocation],
    'Greater Spell Focus (Illusion)': [StandardFeats.GreaterSpellFocus, StandardSchools.Illusion],
    'Greater Spell Focus (Necromancy)': [StandardFeats.GreaterSpellFocus, StandardSchools.Necromancy],
    'Greater Spell Focus (Transmutation)': [StandardFeats.GreaterSpellFocus, StandardSchools.Transmutation],
    'Greater Spell Penetration': StandardFeats.GreaterSpellPenetration,
    'Greater Two Weapon Fighting': StandardFeats.GreaterTwoWeaponFighting,
    'Greater Weapon Focus (Gauntlet)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Gauntlet],
    'Greater Weapon Focus (Unarmed strike - medium-sized being)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.UnarmedStrike],
    'Greater Weapon Focus (Unarmed strike - small being)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.UnarmedStrike],
    'Greater Weapon Focus (Dagger)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Dagger],
    'Greater Weapon Focus (Punching Dagger)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.PunchingDagger],
    'Greater Weapon Focus (Spiked Gauntlet)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.SpikedGauntlet],
    'Greater Weapon Focus (Light Mace)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.LightMace],
    'Greater Weapon Focus (Sickle)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Sickle],
    'Greater Weapon Focus (Club)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Club],
    'Greater Weapon Focus (Shortspear)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Shortspear],
    'Greater Weapon Focus (Heavy mace)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.HeavyMace],
    'Greater Weapon Focus (Morningstar)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Morningstar],
    'Greater Weapon Focus (Quarterstaff)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Quarterstaff],
    'Greater Weapon Focus (Spear)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Spear],
    'Greater Weapon Focus (Light Crossbow)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.LightCrossbow],
    'Greater Weapon Focus (Dart)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Dart],
    'Greater Weapon Focus (Sling)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Sling],
    'Greater Weapon Focus (Heavy Crossbow)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.HeavyCrossbow],
    'Greater Weapon Focus (Javelin)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Javelin],
    'Greater Weapon Focus (Throwing Axe)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.ThrowingAxe],
    'Greater Weapon Focus (Light Hammer)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.LightHammer],
    'Greater Weapon Focus (Handaxe)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Handaxe],
    'Greater Weapon Focus (Light Lance)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Lance],
    'Greater Weapon Focus (Light Pick)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.LightPick],
    'Greater Weapon Focus (Sap)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Sap],
    'Greater Weapon Focus (Short Sword)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.ShortSword],
    'Greater Weapon Focus (Battleaxe)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Battleaxe],
    'Greater Weapon Focus (Light Flail)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Flail],
    'Greater Weapon Focus (Heavy Lance)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Lance],
    'Greater Weapon Focus (Longsword)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Longsword],
    'Greater Weapon Focus (Heavy Pick)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.HeavyPick],
    'Greater Weapon Focus (Rapier)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Rapier],
    'Greater Weapon Focus (Scimitar)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Scimitar],
    'Greater Weapon Focus (Trident)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Trident],
    'Greater Weapon Focus (Warhammer)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Warhammer],
    'Greater Weapon Focus (Falchion)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Falchion],
    'Greater Weapon Focus (Heavy Flail)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.HeavyFlail],
    'Greater Weapon Focus (Glaive)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Glaive],
    'Greater Weapon Focus (Greataxe)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Greataxe],
    'Greater Weapon Focus (Greatclub)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Greatclub],
    'Greater Weapon Focus (Greatsword)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Greatsword],
    'Greater Weapon Focus (Guisarme)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Guisarme],
    'Greater Weapon Focus (Halberd)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Halberd],
    'Greater Weapon Focus (Longspear)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Longspear],
    'Greater Weapon Focus (Ranseur)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Ranseur],
    'Greater Weapon Focus (Scythe)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Scythe],
    'Greater Weapon Focus (Shortbow)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Shortbow],
    'Greater Weapon Focus (Composite Shortbow)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.CompositeShortbow],
    'Greater Weapon Focus (Longbow)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Longbow],
    'Greater Weapon Focus (Composite Longbow)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.CompositeLongbow],
    'Greater Weapon Focus (Halfling Kama)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Kama],
    'Greater Weapon Focus (Kukri)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Kukri],
    'Greater Weapon Focus (Halfling Nunchaku)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Nunchaku],
    'Greater Weapon Focus (Halfling Siangham)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Siangham],
    'Greater Weapon Focus (Kama)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Kama],
    'Greater Weapon Focus (Nunchaku)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Nunchaku],
    'Greater Weapon Focus (Siangham)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Siangham],
    'Greater Weapon Focus (Bastard Sword)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.BastardSword],
    'Greater Weapon Focus (Dwarven Waraxe)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.DwarvenWaraxe],
    'Greater Weapon Focus (Gnome Hooked Hammer)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.HookedGnomeHammer],
    'Greater Weapon Focus (Orc Double Axe)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.OrcDoubleAxe],
    'Greater Weapon Focus (Spike Chain)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.SpikedChain],
    'Greater Weapon Focus (Dire Flail)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.DireFlail],
    'Greater Weapon Focus (Two-bladed Sword)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.TwoBladedSword],
    'Greater Weapon Focus (Dwarven Urgrosh)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.DwarvenUrgrosh],
    'Greater Weapon Focus (Hand Crossbow)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.HandCrossbow],
    'Greater Weapon Focus (Shuriken)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Shuriken],
    'Greater Weapon Focus (Whip)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Whip],
    'Greater Weapon Focus (Repeating Crossbow)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.LightRepeatingCrossbow],
    'Greater Weapon Focus (Net)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Net],
    'Greater Weapon Focus (Grapple)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Grapple],
    'Greater Weapon Focus (Ray)': [StandardFeats.GreaterWeaponFocus, StandardWeapons.Ray],
    'Greater Weapon Specialization': StandardFeats.GreaterWeaponSpecialization,
    'Heighten Spell': StandardFeats.HeightenSpell,
    'Improved Bull Rush': StandardFeats.ImprovedBullRush,
    'Improved Counterspell': StandardFeats.ImprovedCounterspell,
    'Improved Critical (Gauntlet)': [StandardFeats.ImprovedCritical, StandardWeapons.Gauntlet],
    'Improved Critical (Unarmed strike - medium-sized being)': [StandardFeats.ImprovedCritical, StandardWeapons.UnarmedStrike],
    'Improved Critical (Unarmed strike - small being)': [StandardFeats.ImprovedCritical, StandardWeapons.UnarmedStrike],
    'Improved Critical (Dagger)': [StandardFeats.ImprovedCritical, StandardWeapons.Dagger],
    'Improved Critical (Punching Dagger)': [StandardFeats.ImprovedCritical, StandardWeapons.PunchingDagger],
    'Improved Critical (Spiked Gauntlet)': [StandardFeats.ImprovedCritical, StandardWeapons.SpikedGauntlet],
    'Improved Critical (Light Mace)': [StandardFeats.ImprovedCritical, StandardWeapons.LightMace],
    'Improved Critical (Sickle)': [StandardFeats.ImprovedCritical, StandardWeapons.Sickle],
    'Improved Critical (Club)': [StandardFeats.ImprovedCritical, StandardWeapons.Club],
    'Improved Critical (Shortspear)': [StandardFeats.ImprovedCritical, StandardWeapons.Shortspear],
    'Improved Critical (Heavy mace)': [StandardFeats.ImprovedCritical, StandardWeapons.HeavyMace],
    'Improved Critical (Morningstar)': [StandardFeats.ImprovedCritical, StandardWeapons.Morningstar],
    'Improved Critical (Quarterstaff)': [StandardFeats.ImprovedCritical, StandardWeapons.Quarterstaff],
    'Improved Critical (Spear)': [StandardFeats.ImprovedCritical, StandardWeapons.Spear],
    'Improved Critical (Light Crossbow)': [StandardFeats.ImprovedCritical, StandardWeapons.LightCrossbow],
    'Improved Critical (Dart)': [StandardFeats.ImprovedCritical, StandardWeapons.Dart],
    'Improved Critical (Sling)': [StandardFeats.ImprovedCritical, StandardWeapons.Sling],
    'Improved Critical (Heavy Crossbow)': [StandardFeats.ImprovedCritical, StandardWeapons.HeavyCrossbow],
    'Improved Critical (Javelin)': [StandardFeats.ImprovedCritical, StandardWeapons.Javelin],
    'Improved Critical (Throwing Axe)': [StandardFeats.ImprovedCritical, StandardWeapons.ThrowingAxe],
    'Improved Critical (Light Hammer)': [StandardFeats.ImprovedCritical, StandardWeapons.LightHammer],
    'Improved Critical (Handaxe)': [StandardFeats.ImprovedCritical, StandardWeapons.Handaxe],
    'Improved Critical (Light Lance)': [StandardFeats.ImprovedCritical, StandardWeapons.Lance],
    'Improved Critical (Light Pick)': [StandardFeats.ImprovedCritical, StandardWeapons.LightPick],
    'Improved Critical (Sap)': [StandardFeats.ImprovedCritical, StandardWeapons.Sap],
    'Improved Critical (Short Sword)': [StandardFeats.ImprovedCritical, StandardWeapons.ShortSword],
    'Improved Critical (Battleaxe)': [StandardFeats.ImprovedCritical, StandardWeapons.Battleaxe],
    'Improved Critical (Light Flail)': [StandardFeats.ImprovedCritical, StandardWeapons.Flail],
    'Improved Critical (Heavy Lance)': [StandardFeats.ImprovedCritical, StandardWeapons.Lance],
    'Improved Critical (Longsword)': [StandardFeats.ImprovedCritical, StandardWeapons.Longsword],
    'Improved Critical (Heavy Pick)': [StandardFeats.ImprovedCritical, StandardWeapons.HeavyPick],
    'Improved Critical (Rapier)': [StandardFeats.ImprovedCritical, StandardWeapons.Rapier],
    'Improved Critical (Scimitar)': [StandardFeats.ImprovedCritical, StandardWeapons.Scimitar],
    'Improved Critical (Trident)': [StandardFeats.ImprovedCritical, StandardWeapons.Trident],
    'Improved Critical (Warhammer)': [StandardFeats.ImprovedCritical, StandardWeapons.Warhammer],
    'Improved Critical (Falchion)': [StandardFeats.ImprovedCritical, StandardWeapons.Falchion],
    'Improved Critical (Heavy Flail)': [StandardFeats.ImprovedCritical, StandardWeapons.HeavyFlail],
    'Improved Critical (Glaive)': [StandardFeats.ImprovedCritical, StandardWeapons.Glaive],
    'Improved Critical (Greataxe)': [StandardFeats.ImprovedCritical, StandardWeapons.Greataxe],
    'Improved Critical (Greatclub)': [StandardFeats.ImprovedCritical, StandardWeapons.Greatclub],
    'Improved Critical (Greatsword)': [StandardFeats.ImprovedCritical, StandardWeapons.Greatsword],
    'Improved Critical (Guisarme)': [StandardFeats.ImprovedCritical, StandardWeapons.Guisarme],
    'Improved Critical (Halberd)': [StandardFeats.ImprovedCritical, StandardWeapons.Halberd],
    'Improved Critical (Longspear)': [StandardFeats.ImprovedCritical, StandardWeapons.Longspear],
    'Improved Critical (Ranseur)': [StandardFeats.ImprovedCritical, StandardWeapons.Ranseur],
    'Improved Critical (Scythe)': [StandardFeats.ImprovedCritical, StandardWeapons.Scythe],
    'Improved Critical (Shortbow)': [StandardFeats.ImprovedCritical, StandardWeapons.Shortbow],
    'Improved Critical (Composite Shortbow)': [StandardFeats.ImprovedCritical, StandardWeapons.CompositeShortbow],
    'Improved Critical (Longbow)': [StandardFeats.ImprovedCritical, StandardWeapons.Longbow],
    'Improved Critical (Composite Longbow)': [StandardFeats.ImprovedCritical, StandardWeapons.CompositeLongbow],
    'Improved Critical (Halfling Kama)': [StandardFeats.ImprovedCritical, StandardWeapons.Kama],
    'Improved Critical (Kukri)': [StandardFeats.ImprovedCritical, StandardWeapons.Kukri],
    'Improved Critical (Halfling Nunchaku)': [StandardFeats.ImprovedCritical, StandardWeapons.Nunchaku],
    'Improved Critical (Halfling Siangham)': [StandardFeats.ImprovedCritical, StandardWeapons.Siangham],
    'Improved Critical (Kama)': [StandardFeats.ImprovedCritical, StandardWeapons.Kama],
    'Improved Critical (Nunchaku)': [StandardFeats.ImprovedCritical, StandardWeapons.Nunchaku],
    'Improved Critical (Siangham)': [StandardFeats.ImprovedCritical, StandardWeapons.Siangham],
    'Improved Critical (Bastard Sword)': [StandardFeats.ImprovedCritical, StandardWeapons.BastardSword],
    'Improved Critical (Dwarven Waraxe)': [StandardFeats.ImprovedCritical, StandardWeapons.DwarvenWaraxe],
    'Improved Critical (Gnome Hooked Hammer)': [StandardFeats.ImprovedCritical, StandardWeapons.HookedGnomeHammer],
    'Improved Critical (Orc Double Axe)': [StandardFeats.ImprovedCritical, StandardWeapons.OrcDoubleAxe],
    'Improved Critical (Spike Chain)': [StandardFeats.ImprovedCritical, StandardWeapons.SpikedChain],
    'Improved Critical (Dire Flail)': [StandardFeats.ImprovedCritical, StandardWeapons.DireFlail],
    'Improved Critical (Two-bladed Sword)': [StandardFeats.ImprovedCritical, StandardWeapons.TwoBladedSword],
    'Improved Critical (Dwarven Urgrosh)': [StandardFeats.ImprovedCritical, StandardWeapons.DwarvenUrgrosh],
    'Improved Critical (Hand Crossbow)': [StandardFeats.ImprovedCritical, StandardWeapons.HandCrossbow],
    'Improved Critical (Shuriken)': [StandardFeats.ImprovedCritical, StandardWeapons.Shuriken],
    'Improved Critical (Whip)': [StandardFeats.ImprovedCritical, StandardWeapons.Whip],
    'Improved Critical (Repeating Crossbow)': [StandardFeats.ImprovedCritical, StandardWeapons.LightRepeatingCrossbow],
    'Improved Critical (Net)': [StandardFeats.ImprovedCritical, StandardWeapons.Net],
    'Improved Disarm': StandardFeats.ImprovedDisarm,
    'Improved Feint': StandardFeats.ImprovedFeint,
    'Improved Grapple': StandardFeats.ImprovedGrapple,
    'Improved Initiative': StandardFeats.ImprovedInitiative,
    'Improved Overrun': StandardFeats.ImprovedOverrun,
    'Improved Shield Bash': StandardFeats.ImprovedShieldBash,
    'Improved Trip': StandardFeats.ImprovedTrip,
    'Improved Two-Weapon Fighting': StandardFeats.ImprovedTwoWeaponFighting,
    'Improved Turning': StandardFeats.ImprovedTurning,
    'Improved Unarmed Strike': StandardFeats.ImprovedUnarmedStrike,
    'Improved Uncanny Dodge': StandardFeats.ImprovedUncannyDodge,
    'Investigator': StandardFeats.Investigator,
    'Iron Will': StandardFeats.IronWill,
    'Leadership': StandardFeats.Leadership,
    'Lightning Reflexes': StandardFeats.LightningReflexes,
    'Magical Affinity': StandardFeats.MagicalAffinity,
    'Manyshot': StandardFeats.Manyshot,
    'Martial Weapon Proficiency (Throwing Axe)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.ThrowingAxe],
    'Martial Weapon Proficiency (Light Hammer)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.LightHammer],
    'Martial Weapon Proficiency (Handaxe)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Handaxe],
    'Martial Weapon Proficiency (Light Lance)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Lance],
    'Martial Weapon Proficiency (Light Pick)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.LightPick],
    'Martial Weapon Proficiency (Sap)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Sap],
    'Martial Weapon Proficiency (Short Sword)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.ShortSword],
    'Martial Weapon Proficiency (Battleaxe)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Battleaxe],
    'Martial Weapon Proficiency (Light Flail)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Flail],
    'Martial Weapon Proficiency (Heavy Lance)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Lance],
    'Martial Weapon Proficiency (Longsword)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Longsword],
    'Martial Weapon Proficiency (Heavy Pick)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.HeavyPick],
    'Martial Weapon Proficiency (Rapier)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Rapier],
    'Martial Weapon Proficiency (Scimitar)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Scimitar],
    'Martial Weapon Proficiency (Trident)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Trident],
    'Martial Weapon Proficiency (Warhammer)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Warhammer],
    'Martial Weapon Proficiency (Falchion)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Falchion],
    'Martial Weapon Proficiency (Heavy Flail)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.HeavyFlail],
    'Martial Weapon Proficiency (Glaive)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Glaive],
    'Martial Weapon Proficiency (Greataxe)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Greataxe],
    'Martial Weapon Proficiency (Greatclub)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Greatclub],
    'Martial Weapon Proficiency (Greatsword)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Greatsword],
    'Martial Weapon Proficiency (Guisarme)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Guisarme],
    'Martial Weapon Proficiency (Halberd)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Halberd],
    'Martial Weapon Proficiency (Longspear)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Longspear],
    'Martial Weapon Proficiency (Ranseur)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Ranseur],
    'Martial Weapon Proficiency (Scythe)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Scythe],
    'Martial Weapon Proficiency (Shortbow)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Shortbow],
    'Martial Weapon Proficiency (Composite Shortbow)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.CompositeShortbow],
    'Martial Weapon Proficiency (Longbow)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.Longbow],
    'Martial Weapon Proficiency (Composite Longbow)': [StandardFeats.MartialWeaponProficiency, StandardWeapons.CompositeLongbow],
    'Maximize Spell': StandardFeats.MaximizeSpell,
    'Mobility': StandardFeats.Mobility,
    'Natural Spell': StandardFeats.NaturalSpell,
    'Negotiator': StandardFeats.Negotiator,
    'Nimble Fingers': StandardFeats.NimbleFingers,
    'Persuasive': StandardFeats.Persuasive,
    'Point Blank Shot': StandardFeats.PointBlankShot,
    'Power Attack': StandardFeats.PowerAttack,
    'Precise Shot': StandardFeats.PreciseShot,
    'Quick Draw': StandardFeats.QuickDraw,
    'Quicken Spell': StandardFeats.QuickenSpell,
    'Rapid Shot': StandardFeats.RapidShot,
    'Rapid Reload': StandardFeats.RapidReload,
    'Running': StandardFeats.Running,
    'Scribe Scroll': StandardFeats.ScribeScroll,
    'Self Sufficient': StandardFeats.SelfSufficient,
    'Shield Proficiency': StandardFeats.ShieldProficiency,
    'Shot on the Run': StandardFeats.ShotOnTheRun,
    'Silent Spell': StandardFeats.SilentSpell,
    'Simple Weapon Proficiency': StandardFeats.SimpleWeaponProficiency,
    'Skill Focus (Appraise)': [StandardFeats.SkillFocus, StandardSkills.Appraise],
    'Skill Focus (Bluff)': [StandardFeats.SkillFocus, StandardSkills.Bluff],
    'Skill Focus (Concentration)': [StandardFeats.SkillFocus, StandardSkills.Concentration],
    'Skill Focus (Diplomacy)': [StandardFeats.SkillFocus, StandardSkills.Diplomacy],
    'Skill Focus (Disable Device)': [StandardFeats.SkillFocus, StandardSkills.DisableDevice],
    'Skill Focus (Gather Information)': [StandardFeats.SkillFocus, StandardSkills.GatherInformation],
    'Skill Focus (Heal)': [StandardFeats.SkillFocus, StandardSkills.Heal],
    'Skill Focus (Hide)': [StandardFeats.SkillFocus, StandardSkills.Hide],
    'Skill Focus (Intimidate)': [StandardFeats.SkillFocus, StandardSkills.Intimidate],
    'Skill Focus (Knowledge)': [StandardFeats.SkillFocus, StandardSkills.Knowledge],
    'Skill Focus (Listen)': [StandardFeats.SkillFocus, StandardSkills.Listen],
    'Skill Focus (Move Silently)': [StandardFeats.SkillFocus, StandardSkills.MoveSilently],
    'Skill Focus (Open Lock)': [StandardFeats.SkillFocus, StandardSkills.OpenLock],
    'Skill Focus (Search)': [StandardFeats.SkillFocus, StandardSkills.Search],
    'Skill Focus (Sense Motive)': [StandardFeats.SkillFocus, StandardSkills.SenseMotive],
    'Skill Focus (Spellcraft)': [StandardFeats.SkillFocus, StandardSkills.Spellcraft],
    'Skill Focus (Spot)': [StandardFeats.SkillFocus, StandardSkills.Spot],
    'Skill Focus (Tumble)': [StandardFeats.SkillFocus, StandardSkills.Tumble],
    'Skill Focus (Use Device)': [StandardFeats.SkillFocus, StandardSkills.UseMagicDevice],
    'Skill Focus (Survival)': [StandardFeats.SkillFocus, StandardSkills.Survival],
    'Snatch Arrows': StandardFeats.SnatchArrows,
    'Spell Focus (Abjuration)': [StandardFeats.SpellFocus, StandardSchools.Abjuration],
    'Spell Focus (Conjuration)': [StandardFeats.SpellFocus, StandardSchools.Conjuration],
    'Spell Focus (Divination)': [StandardFeats.SpellFocus, StandardSchools.Divination],
    'Spell Focus (Enchantment)': [StandardFeats.SpellFocus, StandardSchools.Enchantment],
    'Spell Focus (Evocation)': [StandardFeats.SpellFocus, StandardSchools.Evocation],
    'Spell Focus (Illusion)': [StandardFeats.SpellFocus, StandardSchools.Illusion],
    'Spell Focus (Necromancy)': [StandardFeats.SpellFocus, StandardSchools.Necromancy],
    'Spell Focus (Transmutation)': [StandardFeats.SpellFocus, StandardSchools.Transmutation],
    'Spell Mastery': StandardFeats.SpellMastery,
    'Spell Penetration': StandardFeats.SpellPenetration,
    'Spring Attack': StandardFeats.SpringAttack,
    'Still Spell': StandardFeats.StillSpell,
    'Stunning Fist': StandardFeats.StunningFist,
    'Stealthy': StandardFeats.Stealthy,
    'Sunder': StandardFeats.Sunder,
    'Toughness': StandardFeats.Toughness,
    'Tower Shield Proficiency': StandardFeats.TowerShieldProficiency,
    'Track': StandardFeats.Track,
    'Trample': StandardFeats.Trample,
    'Two-Weapon Fighting': StandardFeats.TwoWeaponFighting,
    'Two-Weapon Defense': StandardFeats.TwoWeaponDefense,
    'Weapon Finesse (Gauntlet)': [StandardFeats.WeaponFinesse, StandardWeapons.Gauntlet],
    'Weapon Finesse (Unarmed strike - medium-sized being)': [StandardFeats.WeaponFinesse, StandardWeapons.UnarmedStrike],
    'Weapon Finesse (Unarmed strike - small being)': [StandardFeats.WeaponFinesse, StandardWeapons.UnarmedStrike],
    'Weapon Finesse (Dagger)': [StandardFeats.WeaponFinesse, StandardWeapons.Dagger],
    'Weapon Finesse (Punching Dagger)': [StandardFeats.WeaponFinesse, StandardWeapons.PunchingDagger],
    'Weapon Finesse (Spiked Gauntlet)': [StandardFeats.WeaponFinesse, StandardWeapons.SpikedGauntlet],
    'Weapon Finesse (Light Mace)': [StandardFeats.WeaponFinesse, StandardWeapons.LightMace],
    'Weapon Finesse (Sickle)': [StandardFeats.WeaponFinesse, StandardWeapons.Sickle],
    'Weapon Finesse (Club)': [StandardFeats.WeaponFinesse, StandardWeapons.Club],
    'Weapon Finesse (Shortspear)': [StandardFeats.WeaponFinesse, StandardWeapons.Shortspear],
    'Weapon Finesse (Heavy mace)': [StandardFeats.WeaponFinesse, StandardWeapons.HeavyMace],
    'Weapon Finesse (Morningstar)': [StandardFeats.WeaponFinesse, StandardWeapons.Morningstar],
    'Weapon Finesse (Quarterstaff)': [StandardFeats.WeaponFinesse, StandardWeapons.Quarterstaff],
    'Weapon Finesse (Spear)': [StandardFeats.WeaponFinesse, StandardWeapons.Spear],
    'Weapon Finesse (Light Crossbow)': [StandardFeats.WeaponFinesse, StandardWeapons.LightCrossbow],
    'Weapon Finesse (Dart)': [StandardFeats.WeaponFinesse, StandardWeapons.Dart],
    'Weapon Finesse (Sling)': [StandardFeats.WeaponFinesse, StandardWeapons.Sling],
    'Weapon Finesse (Heavy Crossbow)': [StandardFeats.WeaponFinesse, StandardWeapons.HeavyCrossbow],
    'Weapon Finesse (Javelin)': [StandardFeats.WeaponFinesse, StandardWeapons.Javelin],
    'Weapon Finesse (Throwing Axe)': [StandardFeats.WeaponFinesse, StandardWeapons.ThrowingAxe],
    'Weapon Finesse (Light Hammer)': [StandardFeats.WeaponFinesse, StandardWeapons.LightHammer],
    'Weapon Finesse (Handaxe)': [StandardFeats.WeaponFinesse, StandardWeapons.Handaxe],
    'Weapon Finesse (Light Lance)': [StandardFeats.WeaponFinesse, StandardWeapons.Lance],
    'Weapon Finesse (Light Pick)': [StandardFeats.WeaponFinesse, StandardWeapons.LightPick],
    'Weapon Finesse (Sap)': [StandardFeats.WeaponFinesse, StandardWeapons.Sap],
    'Weapon Finesse (Short Sword)': [StandardFeats.WeaponFinesse, StandardWeapons.ShortSword],
    'Weapon Finesse (Battleaxe)': [StandardFeats.WeaponFinesse, StandardWeapons.Battleaxe],
    'Weapon Finesse (Light Flail)': [StandardFeats.WeaponFinesse, StandardWeapons.Flail],
    'Weapon Finesse (Heavy Lance)': [StandardFeats.WeaponFinesse, StandardWeapons.Lance],
    'Weapon Finesse (Longsword)': [StandardFeats.WeaponFinesse, StandardWeapons.Longsword],
    'Weapon Finesse (Heavy Pick)': [StandardFeats.WeaponFinesse, StandardWeapons.HeavyPick],
    'Weapon Finesse (Rapier)': [StandardFeats.WeaponFinesse, StandardWeapons.Rapier],
    'Weapon Finesse (Scimitar)': [StandardFeats.WeaponFinesse, StandardWeapons.Scimitar],
    'Weapon Finesse (Trident)': [StandardFeats.WeaponFinesse, StandardWeapons.Trident],
    'Weapon Finesse (Warhammer)': [StandardFeats.WeaponFinesse, StandardWeapons.Warhammer],
    'Weapon Finesse (Falchion)': [StandardFeats.WeaponFinesse, StandardWeapons.Falchion],
    'Weapon Finesse (Heavy Flail)': [StandardFeats.WeaponFinesse, StandardWeapons.HeavyFlail],
    'Weapon Finesse (Glaive)': [StandardFeats.WeaponFinesse, StandardWeapons.Glaive],
    'Weapon Finesse (Greataxe)': [StandardFeats.WeaponFinesse, StandardWeapons.Greataxe],
    'Weapon Finesse (Greatclub)': [StandardFeats.WeaponFinesse, StandardWeapons.Greatclub],
    'Weapon Finesse (Greatsword)': [StandardFeats.WeaponFinesse, StandardWeapons.Greatsword],
    'Weapon Finesse (Guisarme)': [StandardFeats.WeaponFinesse, StandardWeapons.Guisarme],
    'Weapon Finesse (Halberd)': [StandardFeats.WeaponFinesse, StandardWeapons.Halberd],
    'Weapon Finesse (Longspear)': [StandardFeats.WeaponFinesse, StandardWeapons.Longspear],
    'Weapon Finesse (Ranseur)': [StandardFeats.WeaponFinesse, StandardWeapons.Ranseur],
    'Weapon Finesse (Scythe)': [StandardFeats.WeaponFinesse, StandardWeapons.Scythe],
    'Weapon Finesse (Shortbow)': [StandardFeats.WeaponFinesse, StandardWeapons.Shortbow],
    'Weapon Finesse (Composite Shortbow)': [StandardFeats.WeaponFinesse, StandardWeapons.CompositeShortbow],
    'Weapon Finesse (Longbow)': [StandardFeats.WeaponFinesse, StandardWeapons.Longbow],
    'Weapon Finesse (Composite Longbow)': [StandardFeats.WeaponFinesse, StandardWeapons.CompositeLongbow],
    'Weapon Finesse (Halfling Kama)': [StandardFeats.WeaponFinesse, StandardWeapons.Kama],
    'Weapon Finesse (Kukri)': [StandardFeats.WeaponFinesse, StandardWeapons.Kukri],
    'Weapon Finesse (Halfling Nunchaku)': [StandardFeats.WeaponFinesse, StandardWeapons.Nunchaku],
    'Weapon Finesse (Halfling Siangham)': [StandardFeats.WeaponFinesse, StandardWeapons.Siangham],
    'Weapon Finesse (Kama)': [StandardFeats.WeaponFinesse, StandardWeapons.Kama],
    'Weapon Finesse (Nunchaku)': [StandardFeats.WeaponFinesse, StandardWeapons.Nunchaku],
    'Weapon Finesse (Siangham)': [StandardFeats.WeaponFinesse, StandardWeapons.Siangham],
    'Weapon Finesse (Bastard Sword)': [StandardFeats.WeaponFinesse, StandardWeapons.BastardSword],
    'Weapon Finesse (Dwarven Waraxe)': [StandardFeats.WeaponFinesse, StandardWeapons.DwarvenWaraxe],
    'Weapon Finesse (Gnome Hooked Hammer)': [StandardFeats.WeaponFinesse, StandardWeapons.HookedGnomeHammer],
    'Weapon Finesse (Orc Double Axe)': [StandardFeats.WeaponFinesse, StandardWeapons.OrcDoubleAxe],
    'Weapon Finesse (Spike Chain)': [StandardFeats.WeaponFinesse, StandardWeapons.SpikedChain],
    'Weapon Finesse (Dire Flail)': [StandardFeats.WeaponFinesse, StandardWeapons.DireFlail],
    'Weapon Finesse (Two-bladed Sword)': [StandardFeats.WeaponFinesse, StandardWeapons.TwoBladedSword],
    'Weapon Finesse (Dwarven Urgrosh)': [StandardFeats.WeaponFinesse, StandardWeapons.DwarvenUrgrosh],
    'Weapon Finesse (Hand Crossbow)': [StandardFeats.WeaponFinesse, StandardWeapons.HandCrossbow],
    'Weapon Finesse (Shuriken)': [StandardFeats.WeaponFinesse, StandardWeapons.Shuriken],
    'Weapon Finesse (Whip)': [StandardFeats.WeaponFinesse, StandardWeapons.Whip],
    'Weapon Finesse (Repeating Crossbow)': [StandardFeats.WeaponFinesse, StandardWeapons.LightRepeatingCrossbow],
    'Weapon Finesse (Net)': [StandardFeats.WeaponFinesse, StandardWeapons.Net],
    'Weapon Focus (Gauntlet)': [StandardFeats.WeaponFocus, StandardWeapons.Gauntlet],
    'Weapon Focus (Unarmed strike - medium-sized being)': [StandardFeats.WeaponFocus, StandardWeapons.UnarmedStrike],
    'Weapon Focus (Unarmed strike - small being)': [StandardFeats.WeaponFocus, StandardWeapons.UnarmedStrike],
    'Weapon Focus (Dagger)': [StandardFeats.WeaponFocus, StandardWeapons.Dagger],
    'Weapon Focus (Punching Dagger)': [StandardFeats.WeaponFocus, StandardWeapons.PunchingDagger],
    'Weapon Focus (Spiked Gauntlet)': [StandardFeats.WeaponFocus, StandardWeapons.SpikedGauntlet],
    'Weapon Focus (Light Mace)': [StandardFeats.WeaponFocus, StandardWeapons.LightMace],
    'Weapon Focus (Sickle)': [StandardFeats.WeaponFocus, StandardWeapons.Sickle],
    'Weapon Focus (Club)': [StandardFeats.WeaponFocus, StandardWeapons.Club],
    'Weapon Focus (Shortspear)': [StandardFeats.WeaponFocus, StandardWeapons.Shortspear],
    'Weapon Focus (Heavy mace)': [StandardFeats.WeaponFocus, StandardWeapons.HeavyMace],
    'Weapon Focus (Morningstar)': [StandardFeats.WeaponFocus, StandardWeapons.Morningstar],
    'Weapon Focus (Quarterstaff)': [StandardFeats.WeaponFocus, StandardWeapons.Quarterstaff],
    'Weapon Focus (Spear)': [StandardFeats.WeaponFocus, StandardWeapons.Spear],
    'Weapon Focus (Light Crossbow)': [StandardFeats.WeaponFocus, StandardWeapons.LightCrossbow],
    'Weapon Focus (Dart)': [StandardFeats.WeaponFocus, StandardWeapons.Dart],
    'Weapon Focus (Sling)': [StandardFeats.WeaponFocus, StandardWeapons.Sling],
    'Weapon Focus (Heavy Crossbow)': [StandardFeats.WeaponFocus, StandardWeapons.HeavyCrossbow],
    'Weapon Focus (Javelin)': [StandardFeats.WeaponFocus, StandardWeapons.Javelin],
    'Weapon Focus (Throwing Axe)': [StandardFeats.WeaponFocus, StandardWeapons.ThrowingAxe],
    'Weapon Focus (Light Hammer)': [StandardFeats.WeaponFocus, StandardWeapons.LightHammer],
    'Weapon Focus (Handaxe)': [StandardFeats.WeaponFocus, StandardWeapons.Handaxe],
    'Weapon Focus (Light Lance)': [StandardFeats.WeaponFocus, StandardWeapons.Lance],
    'Weapon Focus (Light Pick)': [StandardFeats.WeaponFocus, StandardWeapons.LightPick],
    'Weapon Focus (Sap)': [StandardFeats.WeaponFocus, StandardWeapons.Sap],
    'Weapon Focus (Short Sword)': [StandardFeats.WeaponFocus, StandardWeapons.ShortSword],
    'Weapon Focus (Battleaxe)': [StandardFeats.WeaponFocus, StandardWeapons.Battleaxe],
    'Weapon Focus (Light Flail)': [StandardFeats.WeaponFocus, StandardWeapons.Flail],
    'Weapon Focus (Heavy Lance)': [StandardFeats.WeaponFocus, StandardWeapons.Lance],
    'Weapon Focus (Longsword)': [StandardFeats.WeaponFocus, StandardWeapons.Longsword],
    'Weapon Focus (Heavy Pick)': [StandardFeats.WeaponFocus, StandardWeapons.HeavyPick],
    'Weapon Focus (Rapier)': [StandardFeats.WeaponFocus, StandardWeapons.Rapier],
    'Weapon Focus (Scimitar)': [StandardFeats.WeaponFocus, StandardWeapons.Scimitar],
    'Weapon Focus (Trident)': [StandardFeats.WeaponFocus, StandardWeapons.Trident],
    'Weapon Focus (Warhammer)': [StandardFeats.WeaponFocus, StandardWeapons.Warhammer],
    'Weapon Focus (Falchion)': [StandardFeats.WeaponFocus, StandardWeapons.Falchion],
    'Weapon Focus (Heavy Flail)': [StandardFeats.WeaponFocus, StandardWeapons.HeavyFlail],
    'Weapon Focus (Glaive)': [StandardFeats.WeaponFocus, StandardWeapons.Glaive],
    'Weapon Focus (Greataxe)': [StandardFeats.WeaponFocus, StandardWeapons.Greataxe],
    'Weapon Focus (Greatclub)': [StandardFeats.WeaponFocus, StandardWeapons.Greatclub],
    'Weapon Focus (Greatsword)': [StandardFeats.WeaponFocus, StandardWeapons.Greatsword],
    'Weapon Focus (Guisarme)': [StandardFeats.WeaponFocus, StandardWeapons.Guisarme],
    'Weapon Focus (Halberd)': [StandardFeats.WeaponFocus, StandardWeapons.Halberd],
    'Weapon Focus (Longspear)': [StandardFeats.WeaponFocus, StandardWeapons.Longspear],
    'Weapon Focus (Ranseur)': [StandardFeats.WeaponFocus, StandardWeapons.Ranseur],
    'Weapon Focus (Scythe)': [StandardFeats.WeaponFocus, StandardWeapons.Scythe],
    'Weapon Focus (Shortbow)': [StandardFeats.WeaponFocus, StandardWeapons.Shortbow],
    'Weapon Focus (Composite Shortbow)': [StandardFeats.WeaponFocus, StandardWeapons.CompositeShortbow],
    'Weapon Focus (Longbow)': [StandardFeats.WeaponFocus, StandardWeapons.Longbow],
    'Weapon Focus (Composite Longbow)': [StandardFeats.WeaponFocus, StandardWeapons.CompositeLongbow],
    'Weapon Focus (Halfling Kama)': [StandardFeats.WeaponFocus, StandardWeapons.Kama],
    'Weapon Focus (Kukri)': [StandardFeats.WeaponFocus, StandardWeapons.Kukri],
    'Weapon Focus (Halfling Nunchaku)': [StandardFeats.WeaponFocus, StandardWeapons.Nunchaku],
    'Weapon Focus (Halfling Siangham)': [StandardFeats.WeaponFocus, StandardWeapons.Siangham],
    'Weapon Focus (Kama)': [StandardFeats.WeaponFocus, StandardWeapons.Kama],
    'Weapon Focus (Nunchaku)': [StandardFeats.WeaponFocus, StandardWeapons.Nunchaku],
    'Weapon Focus (Siangham)': [StandardFeats.WeaponFocus, StandardWeapons.Siangham],
    'Weapon Focus (Bastard Sword)': [StandardFeats.WeaponFocus, StandardWeapons.BastardSword],
    'Weapon Focus (Dwarven Waraxe)': [StandardFeats.WeaponFocus, StandardWeapons.DwarvenWaraxe],
    'Weapon Focus (Gnome Hooked Hammer)': [StandardFeats.WeaponFocus, StandardWeapons.HookedGnomeHammer],
    'Weapon Focus (Orc Double Axe)': [StandardFeats.WeaponFocus, StandardWeapons.OrcDoubleAxe],
    'Weapon Focus (Spike Chain)': [StandardFeats.WeaponFocus, StandardWeapons.SpikedChain],
    'Weapon Focus (Dire Flail)': [StandardFeats.WeaponFocus, StandardWeapons.DireFlail],
    'Weapon Focus (Two-bladed Sword)': [StandardFeats.WeaponFocus, StandardWeapons.TwoBladedSword],
    'Weapon Focus (Dwarven Urgrosh)': [StandardFeats.WeaponFocus, StandardWeapons.DwarvenUrgrosh],
    'Weapon Focus (Hand Crossbow)': [StandardFeats.WeaponFocus, StandardWeapons.HandCrossbow],
    'Weapon Focus (Shuriken)': [StandardFeats.WeaponFocus, StandardWeapons.Shuriken],
    'Weapon Focus (Whip)': [StandardFeats.WeaponFocus, StandardWeapons.Whip],
    'Weapon Focus (Repeating Crossbow)': [StandardFeats.WeaponFocus, StandardWeapons.LightRepeatingCrossbow],
    'Weapon Focus (Net)': [StandardFeats.WeaponFocus, StandardWeapons.Net],
    'Weapon Focus (Grapple)': [StandardFeats.WeaponFocus, StandardWeapons.Grapple],
    'Weapon Focus (Ray)': [StandardFeats.WeaponFocus, StandardWeapons.Ray],
    'Weapon Specialization (Gauntlet)': [StandardFeats.WeaponSpecialization, StandardWeapons.Gauntlet],
    'Weapon Specialization (Unarmed strike - medium-sized being)': [StandardFeats.WeaponSpecialization, StandardWeapons.UnarmedStrike],
    'Weapon Specialization (Unarmed strike - small being)': [StandardFeats.WeaponSpecialization, StandardWeapons.UnarmedStrike],
    'Weapon Specialization (Dagger)': [StandardFeats.WeaponSpecialization, StandardWeapons.Dagger],
    'Weapon Specialization (Punching Dagger)': [StandardFeats.WeaponSpecialization, StandardWeapons.PunchingDagger],
    'Weapon Specialization (Spiked Gauntlet)': [StandardFeats.WeaponSpecialization, StandardWeapons.SpikedGauntlet],
    'Weapon Specialization (Light Mace)': [StandardFeats.WeaponSpecialization, StandardWeapons.LightMace],
    'Weapon Specialization (Sickle)': [StandardFeats.WeaponSpecialization, StandardWeapons.Sickle],
    'Weapon Specialization (Club)': [StandardFeats.WeaponSpecialization, StandardWeapons.Club],
    'Weapon Specialization (Shortspear)': [StandardFeats.WeaponSpecialization, StandardWeapons.Shortspear],
    'Weapon Specialization (Heavy mace)': [StandardFeats.WeaponSpecialization, StandardWeapons.HeavyMace],
    'Weapon Specialization (Morningstar)': [StandardFeats.WeaponSpecialization, StandardWeapons.Morningstar],
    'Weapon Specialization (Quarterstaff)': [StandardFeats.WeaponSpecialization, StandardWeapons.Quarterstaff],
    'Weapon Specialization (Spear)': [StandardFeats.WeaponSpecialization, StandardWeapons.Spear],
    'Weapon Specialization (Throwing Axe)': [StandardFeats.WeaponSpecialization, StandardWeapons.ThrowingAxe],
    'Weapon Specialization (Light Hammer)': [StandardFeats.WeaponSpecialization, StandardWeapons.LightHammer],
    'Weapon Specialization (Handaxe)': [StandardFeats.WeaponSpecialization, StandardWeapons.Handaxe],
    'Weapon Specialization (Light Lance)': [StandardFeats.WeaponSpecialization, StandardWeapons.Lance],
    'Weapon Specialization (Light Pick)': [StandardFeats.WeaponSpecialization, StandardWeapons.LightPick],
    'Weapon Specialization (Sap)': [StandardFeats.WeaponSpecialization, StandardWeapons.Sap],
    'Weapon Specialization (Short Sword)': [StandardFeats.WeaponSpecialization, StandardWeapons.ShortSword],
    'Weapon Specialization (Battleaxe)': [StandardFeats.WeaponSpecialization, StandardWeapons.Battleaxe],
    'Weapon Specialization (Light Flail)': [StandardFeats.WeaponSpecialization, StandardWeapons.Flail],
    'Weapon Specialization (Heavy Lance)': [StandardFeats.WeaponSpecialization, StandardWeapons.Lance],
    'Weapon Specialization (Longsword)': [StandardFeats.WeaponSpecialization, StandardWeapons.Longsword],
    'Weapon Specialization (Heavy Pick)': [StandardFeats.WeaponSpecialization, StandardWeapons.HeavyPick],
    'Weapon Specialization (Rapier)': [StandardFeats.WeaponSpecialization, StandardWeapons.Rapier],
    'Weapon Specialization (Scimitar)': [StandardFeats.WeaponSpecialization, StandardWeapons.Scimitar],
    'Weapon Specialization (Trident)': [StandardFeats.WeaponSpecialization, StandardWeapons.Trident],
    'Weapon Specialization (Warhammer)': [StandardFeats.WeaponSpecialization, StandardWeapons.Warhammer],
    'Weapon Specialization (Falchion)': [StandardFeats.WeaponSpecialization, StandardWeapons.Falchion],
    'Weapon Specialization (Heavy Flail)': [StandardFeats.WeaponSpecialization, StandardWeapons.HeavyFlail],
    'Weapon Specialization (Glaive)': [StandardFeats.WeaponSpecialization, StandardWeapons.Glaive],
    'Weapon Specialization (Greataxe)': [StandardFeats.WeaponSpecialization, StandardWeapons.Greataxe],
    'Weapon Specialization (Greatclub)': [StandardFeats.WeaponSpecialization, StandardWeapons.Greatclub],
    'Weapon Specialization (Greatsword)': [StandardFeats.WeaponSpecialization, StandardWeapons.Greatsword],
    'Weapon Specialization (Guisarme)': [StandardFeats.WeaponSpecialization, StandardWeapons.Guisarme],
    'Weapon Specialization (Halberd)': [StandardFeats.WeaponSpecialization, StandardWeapons.Halberd],
    'Weapon Specialization (Longspear)': [StandardFeats.WeaponSpecialization, StandardWeapons.Longspear],
    'Weapon Specialization (Ranseur)': [StandardFeats.WeaponSpecialization, StandardWeapons.Ranseur],
    'Weapon Specialization (Scythe)': [StandardFeats.WeaponSpecialization, StandardWeapons.Scythe],
    'Weapon Specialization (Shortbow)': [StandardFeats.WeaponSpecialization, StandardWeapons.Shortbow],
    'Weapon Specialization (Composite Shortbow)': [StandardFeats.WeaponSpecialization, StandardWeapons.CompositeShortbow],
    'Weapon Specialization (Longbow)': [StandardFeats.WeaponSpecialization, StandardWeapons.Longbow],
    'Weapon Specialization (Composite Longbow)': [StandardFeats.WeaponSpecialization, StandardWeapons.CompositeLongbow],
    'Weapon Specialization (Halfling Kama)': [StandardFeats.WeaponSpecialization, StandardWeapons.Kama],
    'Weapon Specialization (Kukri)': [StandardFeats.WeaponSpecialization, StandardWeapons.Kukri],
    'Weapon Specialization (Halfling Nunchaku)': [StandardFeats.WeaponSpecialization, StandardWeapons.Nunchaku],
    'Weapon Specialization (Halfling Siangham)': [StandardFeats.WeaponSpecialization, StandardWeapons.Siangham],
    'Weapon Specialization (Kama)': [StandardFeats.WeaponSpecialization, StandardWeapons.Kama],
    'Weapon Specialization (Nunchaku)': [StandardFeats.WeaponSpecialization, StandardWeapons.Nunchaku],
    'Weapon Specialization (Siangham)': [StandardFeats.WeaponSpecialization, StandardWeapons.Siangham],
    'Weapon Specialization (Bastard Sword)': [StandardFeats.WeaponSpecialization, StandardWeapons.BastardSword],
    'Weapon Specialization (Dwarven Waraxe)': [StandardFeats.WeaponSpecialization, StandardWeapons.DwarvenWaraxe],
    'Weapon Specialization (Gnome Hooked Hammer)': [StandardFeats.WeaponSpecialization, StandardWeapons.HookedGnomeHammer],
    'Weapon Specialization (Orc Double Axe)': [StandardFeats.WeaponSpecialization, StandardWeapons.OrcDoubleAxe],
    'Weapon Specialization (Spike Chain)': [StandardFeats.WeaponSpecialization, StandardWeapons.SpikedChain],
    'Weapon Specialization (Dire Flail)': [StandardFeats.WeaponSpecialization, StandardWeapons.DireFlail],
    'Weapon Specialization (Two-bladed Sword)': [StandardFeats.WeaponSpecialization, StandardWeapons.TwoBladedSword],
    'Weapon Specialization (Dwarven Urgrosh)': [StandardFeats.WeaponSpecialization, StandardWeapons.DwarvenUrgrosh],
    'Weapon Specialization (Hand Crossbow)': [StandardFeats.WeaponSpecialization, StandardWeapons.HandCrossbow],
    'Weapon Specialization (Shuriken)': [StandardFeats.WeaponSpecialization, StandardWeapons.Shuriken],
    'Weapon Specialization (Whip)': [StandardFeats.WeaponSpecialization, StandardWeapons.Whip],
    'Weapon Specialization (Repeating Crossbow)': [StandardFeats.WeaponSpecialization, StandardWeapons.LightRepeatingCrossbow],
    'Weapon Specialization (Net)': [StandardFeats.WeaponSpecialization, StandardWeapons.Net],
    'Weapon Specialization (Grapple)': [StandardFeats.WeaponSpecialization, StandardWeapons.Grapple],
    'feat whirlwind attack': StandardFeats.WhirlwindAttack,
    'feat combat reflexes': StandardFeats.CombatReflexes
    /*'feat barbarian rage': StandardFeats.FeatBarbarianRage,
     'feat stunning attacks': StandardFeats.FeatStunningAttacks,
     'feat wholeness of body': StandardFeats.FeatWholenessOfBody,
     'feat lay on hands': StandardFeats.FeatLayOnHands,
     'feat smite evil': StandardFeats.FeatSmiteEvil,
     'feat remove disease': StandardFeats.FeatRemoveDisease,
     'feat detect evil': StandardFeats.FeatDetectEvil,
     'feat aura of courage': StandardFeats.FeatAuraOfCourage,
     'feat divine health': StandardFeats.FeatDivineHealth,
     'feat divine grace': StandardFeats.FeatDivineGrace,
     'feat special mount': StandardFeats.FeatSpecialMount,
     'feat code of conduct': StandardFeats.FeatCodeOfConduct,
     'feat associates': StandardFeats.FeatAssociates,
     'feat defensive roll': StandardFeats.FeatDefensiveRoll,
     'feat turn undead': StandardFeats.FeatTurnUndead,
     'feat rebuke undead': StandardFeats.FeatRebukeUndead,
     'feat domain power': StandardFeats.FeatDomainPower,
     'feat spontaneous casting cure': StandardFeats.FeatSpontaneousCastingCure,
     'feat spontaneous casting inflict': StandardFeats.FeatSpontaneousCastingInflict,
     'feat combat reflexes': StandardFeats.FeatCombatReflexes,
     'feat martial weapon proficiency all': StandardFeats.FeatMartialWeaponProficiencyAll,
     'feat simple weapon proficiency druid': StandardFeats.FeatSimpleWeaponProficiencyDruid,
     'feat simple weapon proficiency monk': StandardFeats.FeatSimpleWeaponProficiencyMonk,
     'feat simple weapon proficiency rogue': StandardFeats.FeatSimpleWeaponProficiencyRogue,
     'feat simple weapon proficiency wizard': StandardFeats.FeatSimpleWeaponProficiencyWizard,
     'feat simple weapon proficiency elf': StandardFeats.FeatSimpleWeaponProficiencyElf,
     'feat uncanny dodge': StandardFeats.FeatUncannyDodge,
     'feat fast movement': StandardFeats.FeatFastMovement,
     'feat bardic music': StandardFeats.FeatBardicMusic,
     'feat bardic knowledge': StandardFeats.FeatBardicKnowledge,
     'feat nature sense': StandardFeats.FeatNatureSense,
     'feat woodland stride': StandardFeats.FeatWoodlandStride,
     'feat trackless step': StandardFeats.FeatTracklessStep,
     'feat resist natures lure': StandardFeats.FeatResistNaturesLure,
     'feat wild shape': StandardFeats.FeatWildShape,
     'feat venom immunity': StandardFeats.FeatVenomImmunity,
     'feat armor proficiency druid': StandardFeats.FeatArmorProficiencyDruid,
     'feat flurry of blows': StandardFeats.FeatFlurryOfBlows,
     'feat evasion': StandardFeats.FeatEvasion,
     'feat still mind': StandardFeats.FeatStillMind,
     'feat purity of body': StandardFeats.FeatPurityOfBody,
     'feat improved evasion': StandardFeats.FeatImprovedEvasion,
     'feat ki strike': StandardFeats.FeatKiStrike,
     'feat sneak attack': StandardFeats.FeatSneakAttack,
     'feat traps': StandardFeats.FeatTraps,
     'feat crippling strike': StandardFeats.FeatCripplingStrike,
     'feat opportunist': StandardFeats.FeatOpportunist,
     'feat skill mastery': StandardFeats.FeatSkillMastery,
     'feat slippery mind': StandardFeats.FeatSlipperyMind,
     'feat call familiar': StandardFeats.FeatCallFamiliar,
     'feat favored enemy aberration': StandardFeats.FeatFavoredEnemyAberration,
     'feat favored enemy animal': StandardFeats.FeatFavoredEnemyAnimal,
     'feat favored enemy beast': StandardFeats.FeatFavoredEnemyBeast,
     'feat favored enemy construct': StandardFeats.FeatFavoredEnemyConstruct,
     'feat favored enemy dragon': StandardFeats.FeatFavoredEnemyDragon,
     'feat favored enemy elemental': StandardFeats.FeatFavoredEnemyElemental,
     'feat favored enemy fey': StandardFeats.FeatFavoredEnemyFey,
     'feat favored enemy giant': StandardFeats.FeatFavoredEnemyGiant,
     'feat favored enemy magical beast': StandardFeats.FeatFavoredEnemyMagicalBeast,
     'feat favored enemy monsterous humanoid': StandardFeats.FeatFavoredEnemyMonsterousHumanoid,
     'feat favored enemy ooze': StandardFeats.FeatFavoredEnemyOoze,
     'feat favored enemy plant': StandardFeats.FeatFavoredEnemyPlant,
     'feat favored enemy shapechanger': StandardFeats.FeatFavoredEnemyShapechanger,
     'feat favored enemy undead': StandardFeats.FeatFavoredEnemyUndead,
     'feat favored enemy vermin': StandardFeats.FeatFavoredEnemyVermin,
     'feat favored enemy outsider evil': StandardFeats.FeatFavoredEnemyOutsiderEvil,
     'feat favored enemy outsider good': StandardFeats.FeatFavoredEnemyOutsiderGood,
     'feat favored enemy outsider lawful': StandardFeats.FeatFavoredEnemyOutsiderLawful,
     'feat favored enemy outsider chaotic': StandardFeats.FeatFavoredEnemyOutsiderChaotic,
     'feat favored enemy humanoid goblinoid': StandardFeats.FeatFavoredEnemyHumanoidGoblinoid,
     'feat favored enemy humanoid reptilian': StandardFeats.FeatFavoredEnemyHumanoidReptilian,
     'feat favored enemy humanoid dwarf': StandardFeats.FeatFavoredEnemyHumanoidDwarf,
     'feat favored enemy humanoid elf': StandardFeats.FeatFavoredEnemyHumanoidElf,
     'feat favored enemy humanoid gnoll': StandardFeats.FeatFavoredEnemyHumanoidGnoll,
     'feat favored enemy humanoid gnome': StandardFeats.FeatFavoredEnemyHumanoidGnome,
     'feat favored enemy humanoid halfling': StandardFeats.FeatFavoredEnemyHumanoidHalfling,
     'feat favored enemy humanoid orc': StandardFeats.FeatFavoredEnemyHumanoidOrc,
     'feat favored enemy humanoid human': StandardFeats.FeatFavoredEnemyHumanoidHuman,
     'feat ambidexterity ranger': StandardFeats.FeatAmbidexterityRanger,
     'feat two weapon fighting ranger': StandardFeats.FeatTwoWeaponFightingRanger,
     'feat improved two weapon fighting ranger': StandardFeats.FeatImprovedTwoWeaponFightingRanger,
     'feat animal companion': StandardFeats.FeatAnimalCompanion,
     'feat ranger two weapon style': StandardFeats.FeatRangerTwoWeaponStyle,
     'feat ranger archery style': StandardFeats.FeatRangerArcheryStyle,
     'feat widen spell': StandardFeats.FeatWidenSpell,
     'feat ranger rapid shot': StandardFeats.FeatRangerRapidShot,
     'feat ranger manyshot': StandardFeats.FeatRangerManyshot,*/
};

function postprocess(prototypes) {

    processLegacyMapList();
    processJumpPoints();
    processReputations();
    processAddMeshes();
    processPortraits();
    processInventory();

    print("Running postprocessor on: " + prototypes);

    for (var k in prototypes) {
        var processor = processors[k];
        var proto = prototypes[k];
        if (processor !== undefined) {
            processor(proto);
        }

        // Generic post-processing
        if (proto['wearMeshId'] !== undefined) {
            proto['wearMeshId'] /= 100; // the last two digits are the specific model-types, but we organize differently
        }
        if (proto['hairType'] !== undefined) {
            var hairId = hairTypes[proto['hairType']];
            if (hairId === undefined) {
                    print("Unknown hair style: " + proto['hairType']);
                    proto['hairType'] = undefined;
            } else {
                    proto['hairType'] = hairId;
            }
        }
        if (proto['hairColor'] !== undefined) {
            var hairId = hairColors[proto['hairColor']];
            if (hairId === undefined) {
                    print("Unknown hair color: " + proto['hairColor']);
                    proto['hairColor'] = undefined;
            } else {
                    proto['hairColor'] = hairId;
            }
        }
        if (proto['portraitId'] !== undefined) {
            var portraitId = proto['portraitId'];
            delete proto['portraitId'];
            proto['portrait'] = Math.floor(portraitId / 10);
        }

        if (proto.alignment) {
            // Map the alignment to the new constants
            proto.alignment = AlignmentMap[proto.alignment];
        }

        if (proto.deity) {
            if (!DeityMap[proto.deity]) {
                print("WARNING: Unknown deity: " + proto.deity);
            }
            proto.deity = DeityMap[proto.deity];
        }

        if (proto.skills) {
            var newSkills = {};
            for (var k in proto.skills) {
                if (!SkillMap[k]) {
                    print("WARNING: Unknown skill: " + k);
                } else {
                    newSkills[SkillMap[k]] = proto.skills[k];
                }
            }
            proto.skills = newSkills;
        }

        if (proto.feats) {
            var newFeats = [];
            proto.feats.forEach(function (feat) {
                if (!FeatMapping[feat]) {
                    print("WARNING: Unknown feat: " + feat);
                } else {
                    newFeats.push(FeatMapping[feat]);
                }
            });
            proto.feats = newFeats;
        }

        var addMeshId = proto['addMeshId'];
        if (addMeshId !== undefined) {
            delete proto['addMeshId'];
            if (addMeshId < 10000000) {
                    print("Warning: Prototype has invalid add mesh id: " + addMeshId);
            } else {
                    var filename = npcAddMeshes[addMeshId];
                    if (filename === undefined) {
                            print ('Unknown addmesh id: ' + addMeshId);
                    } else {
                            proto['addMeshes'] = [filename];
                    }
            }
        }
    }

    var result = JSON.stringify(prototypes);
    addFile('prototypes.js', result, 9);

    print("Finished postprocessing");

    return result;
}

function processLegacyMapList() {
    var records = readMes('rules/MapList.mes');

    for (var mapId in records) {
        var mapDir = records[mapId].split(',')[0];
        LegacyMapList[mapId] = mapDir.toLowerCase();
    }
}

function mangleFilename(filename) {
        filename = filename.replace(/\\/g, '/');
        filename = filename.replace(/^art\/+/i, '');
        return filename;
}

function processAddMeshes() {
        var addMeshes = readMes('rules/addmesh.mes');

        var equipment = {};

        var typeIds = {};
        typeIds[0] = 'human-male';
        typeIds[1] = 'human-female';
        typeIds[2] = 'elf-male';
        typeIds[3] = 'elf-female';
        typeIds[4] = 'halforc-male';
        typeIds[5] = 'halforc-female';
        typeIds[6] = 'dwarf-male';
        typeIds[7] = 'dwarf-female';
        typeIds[8] = 'gnome-male';
        typeIds[9] = 'gnome-female';
        typeIds[10] = 'halfelf-male';
        typeIds[11] = 'halfelf-female';
        typeIds[12] = 'halfling-male';
        typeIds[13] = 'halfling-female';

        // Group by item-id
        for (var k in addMeshes) {
                if (k >= 10000000) {
                        var filename = addMeshes[k];
                        filename = filename.replace(/\\/g, '/');
                        filename = filename.replace(/^art\/+/i, '');
                        filename = filename.replace(/skm$/i, 'model');
                        npcAddMeshes[k] = filename;
                        continue;
                }

                var id = Math.floor(k / 100);
                var typeId = typeIds[k % 100];

                if (typeId === undefined) {
                        print("Unknown type id for addmesh: " + typeId);
                }

                if (equipment[id] === undefined)
                        equipment[id] = {};

                if (equipment[id][typeId] === undefined)
                        equipment[id][typeId] = {};

                var meshFilenames = addMeshes[k].split(';');

                for (var i = 0; i < meshFilenames.length; ++i) {
                        meshFilenames[i] = meshFilenames[i].replace(/\\/g, '/');
                        meshFilenames[i] = meshFilenames[i].replace(/^art\/+/i, '');
                        meshFilenames[i] = meshFilenames[i].replace(/skm$/i, 'model');
                }

                equipment[id][typeId].meshes = meshFilenames;
        }

        // Group by item-id
        var materials = readMes('rules/materials.mes');
        for (var k in materials) {
                var id = Math.floor(k / 100);
                var typeId = typeIds[k % 100];

                if (typeId === undefined) {
                        print("Unknown type id for material: " + typeId);
                }

                if (equipment[id] === undefined)
                        equipment[id] = {};

                if (equipment[id][typeId] === undefined)
                        equipment[id][typeId] = {};

                var line = materials[k].split(':');
                var slot = line[0];
                var material = mangleFilename(line[1]).replace(/mdf$/i, 'xml');

                if (equipment[id][typeId].materials === undefined)
                        equipment[id][typeId].materials = {};
                equipment[id][typeId].materials[slot] = material;
        }

        var result = JSON.stringify(equipment);
        addFile('equipment.js', result, 9);
}

function processJumpPoints() {
        var records = readTab('rules/jumppoint.tab');

        var jumppoints = {};

        for (var i = 0; i < records.length; ++i) {
                var record = records[i];

                var mapId = LegacyMapList[record[2]];

                if (mapId === undefined) {
                        print("Warning: Undefined map id in jumppoint " + record[0] + ": " + record[2]);
                        continue;
                }

                var jumppoint = {
                        name: record[1],
                        map: mapId,
                        position: [Math.round((parseInt(record[3]) + 0.5) * PixelPerWorldTile),
                                   0,
                                   Math.round((parseInt(record[4]) + 0.5) * PixelPerWorldTile)
                        ]
                };

                jumppoints[record[0]] = jumppoint;
        }

        var result = JSON.stringify(jumppoints);
        addFile('jumppoints.js', result, 9);
}

function processReputations() {
    var records = readMes('mes/gamereplog.mes');

    var reputations = {};

    for (var i = 0; i < 1000; ++i) {
        var name = records[i];
        if (!name)
            continue;

        reputations[i] = {
            'name': name,
            'description': records[1000 + i],
            'effect': records[2000 + i]
        };
    }

    var result = JSON.stringify(reputations);
    addFile('reputations.js', result, 9);
}

function processPortraits() {
    var records = readMes('art/interface/portraits/portraits.mes');

    var portraits = [];

    var PortraitPattern = /^(\w\w)([mf])_.*/i;

    for (var k in records) {
        k = parseInt(k);

        if (k % 10 != 0)
                continue;

        var large = records[k];
        if (large == '')
                large = null;
        var medium = records[k + 1];
        var small = records[k + 2];
        var smallGray = records[k + 3];
        var mediumGray = records[k + 4];

        var files = [large, medium, small, mediumGray, smallGray];

        for (var i = 0; i < files.length; ++i) {
            if (files[i] === undefined || files[i] === null)
                    continue;
            files[i] = files[i].replace(/\.tga$/i, '.png');
        }

        var portrait = {
            id: k/10,
            large: files[0],
            medium: files[1],
            small: files[2],
            mediumGray: files[3],
            smallGray: files[4]
        };

        var result = PortraitPattern.exec(files[0]);

        if (result) {
            switch (result[1].toLowerCase()) {
            case 'hu':
                portrait.race = 'human';
                break;
            case 'el':
                portrait.race = 'elf';
                break;
            case 'he':
                portrait.race = 'halfelf';
                break;
            case 'dw':
                portrait.race = 'dwarf';
                break;
            case 'ha':
                portrait.race = 'halfling';
                break;
            case 'ho':
                portrait.race = 'halforc';
                break;
            case 'gn':
                portrait.race = 'gnome';
                break;
            };
            switch (result[2].toLowerCase()) {
            case 'm':
                portrait.gender = 'male';
                break;
            case 'f':
                portrait.gender ='female';
                break;
            };
        }

        portraits.push(portrait);
    }

    var result = JSON.stringify(portraits);
    addFile('portraits.js', result, 9);
}

function processInventory() {
        var records = readMes('art/interface/inventory/inventory.mes');

        var inventory = {};

        for (var k in records) {
                inventory[k] = records[k].replace(/\.tga$/i, '.png');
        }

        var result = JSON.stringify(inventory);
        addFile('inventoryIcons.js', result, 9);
}

/*
    http://www.JSON.org/json2.js
    2010-03-20

    Public Domain.

    NO WARRANTY EXPRESSED OR IMPLIED. USE AT YOUR OWN RISK.

    See http://www.JSON.org/js.html


    This code should be minified before deployment.
    See http://javascript.crockford.com/jsmin.html

    USE YOUR OWN COPY. IT IS EXTREMELY UNWISE TO LOAD CODE FROM SERVERS YOU DO
    NOT CONTROL.


    This file creates a global JSON object containing two methods: stringify
    and parse.

        JSON.stringify(value, replacer, space)
            value       any JavaScript value, usually an object or array.

            replacer    an optional parameter that determines how object
                        values are stringified for objects. It can be a
                        function or an array of strings.

            space       an optional parameter that specifies the indentation
                        of nested structures. If it is omitted, the text will
                        be packed without extra whitespace. If it is a number,
                        it will specify the number of spaces to indent at each
                        level. If it is a string (such as '\t' or '&nbsp;'),
                        it contains the characters used to indent at each level.

            This method produces a JSON text from a JavaScript value.

            When an object value is found, if the object contains a toJSON
            method, its toJSON method will be called and the result will be
            stringified. A toJSON method does not serialize: it returns the
            value represented by the name/value pair that should be serialized,
            or undefined if nothing should be serialized. The toJSON method
            will be passed the key associated with the value, and this will be
            bound to the value

            For example, this would serialize Dates as ISO strings.

                Date.prototype.toJSON = function (key) {
                    function f(n) {
                        // Format integers to have at least two digits.
                        return n < 10 ? '0' + n : n;
                    }

                    return this.getUTCFullYear()   + '-' +
                         f(this.getUTCMonth() + 1) + '-' +
                         f(this.getUTCDate())      + 'T' +
                         f(this.getUTCHours())     + ':' +
                         f(this.getUTCMinutes())   + ':' +
                         f(this.getUTCSeconds())   + 'Z';
                };

            You can provide an optional replacer method. It will be passed the
            key and value of each member, with this bound to the containing
            object. The value that is returned from your method will be
            serialized. If your method returns undefined, then the member will
            be excluded from the serialization.

            If the replacer parameter is an array of strings, then it will be
            used to select the members to be serialized. It filters the results
            such that only members with keys listed in the replacer array are
            stringified.

            Values that do not have JSON representations, such as undefined or
            functions, will not be serialized. Such values in objects will be
            dropped; in arrays they will be replaced with null. You can use
            a replacer function to replace those with JSON values.
            JSON.stringify(undefined) returns undefined.

            The optional space parameter produces a stringification of the
            value that is filled with line breaks and indentation to make it
            easier to read.

            If the space parameter is a non-empty string, then that string will
            be used for indentation. If the space parameter is a number, then
            the indentation will be that many spaces.

            Example:

            text = JSON.stringify(['e', {pluribus: 'unum'}]);
            // text is '["e",{"pluribus":"unum"}]'


            text = JSON.stringify(['e', {pluribus: 'unum'}], null, '\t');
            // text is '[\n\t"e",\n\t{\n\t\t"pluribus": "unum"\n\t}\n]'

            text = JSON.stringify([new Date()], function (key, value) {
                return this[key] instanceof Date ?
                    'Date(' + this[key] + ')' : value;
            });
            // text is '["Date(---current time---)"]'


        JSON.parse(text, reviver)
            This method parses a JSON text to produce an object or array.
            It can throw a SyntaxError exception.

            The optional reviver parameter is a function that can filter and
            transform the results. It receives each of the keys and values,
            and its return value is used instead of the original value.
            If it returns what it received, then the structure is not modified.
            If it returns undefined then the member is deleted.

            Example:

            // Parse the text. Values that look like ISO date strings will
            // be converted to Date objects.

            myData = JSON.parse(text, function (key, value) {
                var a;
                if (typeof value === 'string') {
                    a =
/^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2}(?:\.\d*)?)Z$/.exec(value);
                    if (a) {
                        return new Date(Date.UTC(+a[1], +a[2] - 1, +a[3], +a[4],
                            +a[5], +a[6]));
                    }
                }
                return value;
            });

            myData = JSON.parse('["Date(09/09/2001)"]', function (key, value) {
                var d;
                if (typeof value === 'string' &&
                        value.slice(0, 5) === 'Date(' &&
                        value.slice(-1) === ')') {
                    d = new Date(value.slice(5, -1));
                    if (d) {
                        return d;
                    }
                }
                return value;
            });


    This is a reference implementation. You are free to copy, modify, or
    redistribute.
*/

/*jslint evil: true, strict: false */

/*members "", "\b", "\t", "\n", "\f", "\r", "\"", JSON, "\\", apply,
    call, charCodeAt, getUTCDate, getUTCFullYear, getUTCHours,
    getUTCMinutes, getUTCMonth, getUTCSeconds, hasOwnProperty, join,
    lastIndex, length, parse, prototype, push, replace, slice, stringify,
    test, toJSON, toString, valueOf
*/


// Create a JSON object only if one does not already exist. We create the
// methods in a closure to avoid creating global variables.

if (!this.JSON) {
    this.JSON = {};
}

(function () {

    function f(n) {
        // Format integers to have at least two digits.
        return n < 10 ? '0' + n : n;
    }

    if (typeof Date.prototype.toJSON !== 'function') {

        Date.prototype.toJSON = function (key) {

            return isFinite(this.valueOf()) ?
                   this.getUTCFullYear()   + '-' +
                 f(this.getUTCMonth() + 1) + '-' +
                 f(this.getUTCDate())      + 'T' +
                 f(this.getUTCHours())     + ':' +
                 f(this.getUTCMinutes())   + ':' +
                 f(this.getUTCSeconds())   + 'Z' : null;
        };

        String.prototype.toJSON =
        Number.prototype.toJSON =
        Boolean.prototype.toJSON = function (key) {
            return this.valueOf();
        };
    }

    var cx = /[\u0000\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,
        escapable = /[\\\"\x00-\x1f\x7f-\x9f\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,
        gap,
        indent,
        meta = {    // table of character substitutions
            '\b': '\\b',
            '\t': '\\t',
            '\n': '\\n',
            '\f': '\\f',
            '\r': '\\r',
            '"' : '\\"',
            '\\': '\\\\'
        },
        rep;


    function quote(string) {

// If the string contains no control characters, no quote characters, and no
// backslash characters, then we can safely slap some quotes around it.
// Otherwise we must also replace the offending characters with safe escape
// sequences.

        escapable.lastIndex = 0;
        return escapable.test(string) ?
            '"' + string.replace(escapable, function (a) {
                var c = meta[a];
                return typeof c === 'string' ? c :
                    '\\u' + ('0000' + a.charCodeAt(0).toString(16)).slice(-4);
            }) + '"' :
            '"' + string + '"';
    }


    function str(key, holder) {

// Produce a string from holder[key].

        var i,          // The loop counter.
            k,          // The member key.
            v,          // The member value.
            length,
            mind = gap,
            partial,
            value = holder[key];

// If the value has a toJSON method, call it to obtain a replacement value.

        if (value && typeof value === 'object' &&
                typeof value.toJSON === 'function') {
            value = value.toJSON(key);
        }

// If we were called with a replacer function, then call the replacer to
// obtain a replacement value.

        if (typeof rep === 'function') {
            value = rep.call(holder, key, value);
        }

// What happens next depends on the value's type.

        switch (typeof value) {
        case 'string':
            return quote(value);

        case 'number':

// JSON numbers must be finite. Encode non-finite numbers as null.

            return isFinite(value) ? String(value) : 'null';

        case 'boolean':
        case 'null':

// If the value is a boolean or null, convert it to a string. Note:
// typeof null does not produce 'null'. The case is included here in
// the remote chance that this gets fixed someday.

            return String(value);

// If the type is 'object', we might be dealing with an object or an array or
// null.

        case 'object':

// Due to a specification blunder in ECMAScript, typeof null is 'object',
// so watch out for that case.

            if (!value) {
                return 'null';
            }

// Make an array to hold the partial results of stringifying this object value.

            gap += indent;
            partial = [];

// Is the value an array?

            if (Object.prototype.toString.apply(value) === '[object Array]') {

// The value is an array. Stringify every element. Use null as a placeholder
// for non-JSON values.

                length = value.length;
                for (i = 0; i < length; i += 1) {
                    partial[i] = str(i, value) || 'null';
                }

// Join all of the elements together, separated with commas, and wrap them in
// brackets.

                v = partial.length === 0 ? '[]' :
                    gap ? '[\n' + gap +
                            partial.join(',\n' + gap) + '\n' +
                                mind + ']' :
                          '[' + partial.join(',') + ']';
                gap = mind;
                return v;
            }

// If the replacer is an array, use it to select the members to be stringified.

            if (rep && typeof rep === 'object') {
                length = rep.length;
                for (i = 0; i < length; i += 1) {
                    k = rep[i];
                    if (typeof k === 'string') {
                        v = str(k, value);
                        if (v) {
                            partial.push(quote(k) + (gap ? ': ' : ':') + v);
                        }
                    }
                }
            } else {

// Otherwise, iterate through all of the keys in the object.

                for (k in value) {
                    if (Object.hasOwnProperty.call(value, k)) {
                        v = str(k, value);
                        if (v) {
                            partial.push(quote(k) + (gap ? ': ' : ':') + v);
                        }
                    }
                }
            }

// Join all of the member texts together, separated with commas,
// and wrap them in braces.

            v = partial.length === 0 ? '{}' :
                gap ? '{\n' + gap + partial.join(',\n' + gap) + '\n' +
                        mind + '}' : '{' + partial.join(',') + '}';
            gap = mind;
            return v;
        }
    }

// If the JSON object does not yet have a stringify method, give it one.

    if (typeof JSON.stringify !== 'function') {
        JSON.stringify = function (value, replacer, space) {

// The stringify method takes a value and an optional replacer, and an optional
// space parameter, and returns a JSON text. The replacer can be a function
// that can replace values, or an array of strings that will select the keys.
// A default replacer method can be provided. Use of the space parameter can
// produce text that is more easily readable.

            var i;
            gap = '';
            indent = '';

// If the space parameter is a number, make an indent string containing that
// many spaces.

            if (typeof space === 'number') {
                for (i = 0; i < space; i += 1) {
                    indent += ' ';
                }

// If the space parameter is a string, it will be used as the indent string.

            } else if (typeof space === 'string') {
                indent = space;
            }

// If there is a replacer, it must be a function or an array.
// Otherwise, throw an error.

            rep = replacer;
            if (replacer && typeof replacer !== 'function' &&
                    (typeof replacer !== 'object' ||
                     typeof replacer.length !== 'number')) {
                throw new Error('JSON.stringify');
            }

// Make a fake root object containing our value under the key of ''.
// Return the result of stringifying the value.

            return str('', {'': value});
        };
    }


// If the JSON object does not yet have a parse method, give it one.

    if (typeof JSON.parse !== 'function') {
        JSON.parse = function (text, reviver) {

// The parse method takes a text and an optional reviver function, and returns
// a JavaScript value if the text is a valid JSON text.

            var j;

            function walk(holder, key) {

// The walk method is used to recursively walk the resulting structure so
// that modifications can be made.

                var k, v, value = holder[key];
                if (value && typeof value === 'object') {
                    for (k in value) {
                        if (Object.hasOwnProperty.call(value, k)) {
                            v = walk(value, k);
                            if (v !== undefined) {
                                value[k] = v;
                            } else {
                                delete value[k];
                            }
                        }
                    }
                }
                return reviver.call(holder, key, value);
            }


// Parsing happens in four stages. In the first stage, we replace certain
// Unicode characters with escape sequences. JavaScript handles many characters
// incorrectly, either silently deleting them, or treating them as line endings.

            text = String(text);
            cx.lastIndex = 0;
            if (cx.test(text)) {
                text = text.replace(cx, function (a) {
                    return '\\u' +
                        ('0000' + a.charCodeAt(0).toString(16)).slice(-4);
                });
            }

// In the second stage, we run the text against regular expressions that look
// for non-JSON patterns. We are especially concerned with '()' and 'new'
// because they can cause invocation, and '=' because it can cause mutation.
// But just to be safe, we want to reject all unexpected forms.

// We split the second stage into 4 regexp operations in order to work around
// crippling inefficiencies in IE's and Safari's regexp engines. First we
// replace the JSON backslash pairs with '@' (a non-JSON character). Second, we
// replace all simple value tokens with ']' characters. Third, we delete all
// open brackets that follow a colon or comma or that begin the text. Finally,
// we look to see that the remaining characters are only whitespace or ']' or
// ',' or ':' or '{' or '}'. If that is so, then the text is safe for eval.

            if (/^[\],:{}\s]*$/.
test(text.replace(/\\(?:["\\\/bfnrt]|u[0-9a-fA-F]{4})/g, '@').
replace(/"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g, ']').
replace(/(?:^|:|,)(?:\s*\[)+/g, ''))) {

// In the third stage we use the eval function to compile the text into a
// JavaScript structure. The '{' operator is subject to a syntactic ambiguity
// in JavaScript: it can begin a block or an object literal. We wrap the text
// in parens to eliminate the ambiguity.

                j = eval('(' + text + ')');

// In the optional fourth stage, we recursively walk the new structure, passing
// each name/value pair to a reviver function for possible transformation.

                return typeof reviver === 'function' ?
                    walk({'': j}, '') : j;
            }

// If the text is not JSON parseable, then a SyntaxError is thrown.

            throw new SyntaxError('JSON.parse');
        };
    }
}());
