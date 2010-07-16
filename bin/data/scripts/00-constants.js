
// Constants for mouse buttons
var Mouse = {
    NoButton: 0,
    LeftButton: 1,
    RightButton: 2,
    MidButton: 4,
    XButton1: 8,
    XButton2: 16
};

/**
 * NPC reaction towards the player
 */
var Reaction = {
    Great: 0,
    Good: 1,
    Neutral: 2,
    Bad: 3,
    Worst: 4
};

/*
    Equipment slot constants
 */
var Slot_Helm = 200;
var Slot_Neck = 201;
var Slot_Gloves = 202;
var Slot_MainHand = 203;
var Slot_OffHand = 204;
var Slot_Chest = 205;
var Slot_FirstRing = 206;
var Slot_SecondRing = 207;
var Slot_Feet = 208;
var Slot_Ammo = 209;
var Slot_Back = 210;
var Slot_Shield = 211;
var Slot_Robes = 212;
var Slot_Bracers = 213;
var Slot_Instrument = 214;
var Slot_Lockpicks = 215;

/*
 Genders.
 */
var Female = 'female';
var Male = 'male';


/**
 * Alignments can be checked via the == operator, but no numeric relation should be assumed.
 * This is why they're objects and not numbers.
 */
var Alignment = {
    LawfulGood: {},
    NeutralGood: {},
    ChaoticGood: {},
    LawfulNeutral: {},
    TrueNeutral: {},
    ChaoticNeutral: {},
    LawfulEvil: {},
    NeutralEvil: {},
    ChaoticEvil: {}
};

var Area = {
    Hommlet: "hommlet",
    Moathouse: "moathouse",
    Nulb: "nulb",
    Temple: "temple",
    EmridyMeadows: "emridy_meadows",
    ImerydsRun: "imeryds_run",
    TempleSecretExit: "temple_secret_exit",
    MoathouseSecretExit: "moathouse_secret_exit",
    OgreCave: "ogre_cave",
    DekloGrove: "deklo_grove",
    TempleRuinedHouse: "temple_ruined_house",
    TempleTower: "temple_tower"
};
