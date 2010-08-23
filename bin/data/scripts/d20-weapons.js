/**
 * Enumerates ids of standard weapon types provided by the D20 SRD. These are used by the weapon profiencies.
 */
var StandardWeapons = {
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

var StandardWeaponCategories = {
    Simple: 'simple',
    Martial: 'martial',
    Exotic: 'exotic'
};

(function() {

    function registerWeapons() {
        Weapons.register({
            id: StandardWeapons.Gauntlet,
            name: qsTr('Gauntlet'),
            category: StandardWeaponCategories.Simple,
            light: true,
            unarmed: true
        });

        Weapons.register({
            id: StandardWeapons.UnarmedStrike,
            name: qsTr('Unarmed Strike'),
            category: StandardWeaponCategories.Simple,
            light: true,
            unarmed: true
        });

        Weapons.register({
            id: StandardWeapons.Dagger,
            name: qsTr(''),
            category: StandardWeaponCategories.Simple,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.PunchingDagger,
            name: qsTr('Punching Dagger'),
            category: StandardWeaponCategories.Simple,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.SpikedGauntlet,
            name: qsTr('Spiked Gauntlet'),
            category: StandardWeaponCategories.Simple,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.LightMace,
            name: qsTr('Light Mace'),
            category: StandardWeaponCategories.Simple,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.Sickle,
            name: qsTr('Sickle'),
            category: StandardWeaponCategories.Simple,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.Club,
            name: qsTr('Club'),
            category: StandardWeaponCategories.Simple
        });

        Weapons.register({
            id: StandardWeapons.HeavyMace,
            name: qsTr('Heavy Mace'),
            category: StandardWeaponCategories.Simple
        });

        Weapons.register({
            id: StandardWeapons.Morningstar,
            name: qsTr('Morningstar'),
            category: StandardWeaponCategories.Simple
        });

        Weapons.register({
            id: StandardWeapons.Shortspear,
            name: qsTr('Shortspear'),
            category: StandardWeaponCategories.Simple
        });

        Weapons.register({
            id: StandardWeapons.Longspear,
            name: qsTr('Longspear'),
            category: StandardWeaponCategories.Simple,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Quarterstaff,
            name: qsTr('Quarterstaff'),
            category: StandardWeaponCategories.Simple
            ,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Spear,
            name: qsTr('Spear'),
            category: StandardWeaponCategories.Simple,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.HeavyCrossbow,
            name: qsTr('Heavy Crossbow'),
            category: StandardWeaponCategories.Simple,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.LightCrossbow,
            name: qsTr('Light Crossbow'),
            category: StandardWeaponCategories.Simple,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.Dart,
            name: qsTr('Dart'),
            category: StandardWeaponCategories.Simple,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.Javelin,
            name: qsTr('Javelin'),
            category: StandardWeaponCategories.Simple,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.Sling,
            name: qsTr('Sling'),
            category: StandardWeaponCategories.Simple,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.ThrowingAxe,
            name: qsTr('Throwing Axe'),
            category: StandardWeaponCategories.Martial,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.LightHammer,
            name: qsTr('Light Hammer'),
            category: StandardWeaponCategories.Martial,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.Handaxe,
            name: qsTr('Handaxe'),
            category: StandardWeaponCategories.Martial,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.Kukri,
            name: qsTr('Kukri'),
            category: StandardWeaponCategories.Martial,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.LightPick,
            name: qsTr('Light Pick'),
            category: StandardWeaponCategories.Martial,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.Sap,
            name: qsTr('Sap'),
            category: StandardWeaponCategories.Martial,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.LightShield,
            name: qsTr('Light Shield'),
            category: StandardWeaponCategories.Martial,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.SpikedArmor,
            name: qsTr('Spiked Armor'),
            category: StandardWeaponCategories.Martial,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.LightSpikedShield,
            name: qsTr('Light Spiked Shield'),
            category: StandardWeaponCategories.Martial,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.ShortSword,
            name: qsTr('Short Sword'),
            category: StandardWeaponCategories.Martial,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.Battleaxe,
            name: qsTr('Battleaxe'),
            category: StandardWeaponCategories.Martial
        });

        Weapons.register({
            id: StandardWeapons.Flail,
            name: qsTr('Flail'),
            category: StandardWeaponCategories.Martial
        });

        Weapons.register({
            id: StandardWeapons.Longsword,
            name: qsTr('Longsword'),
            category: StandardWeaponCategories.Martial
        });

        Weapons.register({
            id: StandardWeapons.HeavyPick,
            name: qsTr('Heavy Pick'),
            category: StandardWeaponCategories.Martial
        });

        Weapons.register({
            id: StandardWeapons.Rapier,
            name: qsTr('Rapier'),
            category: StandardWeaponCategories.Martial,
            prototype: 4009
        });

        Weapons.register({
            id: StandardWeapons.Scimitar,
            name: qsTr('Scimitar'),
            category: StandardWeaponCategories.Martial
        });

        Weapons.register({
            id: StandardWeapons.HeavyShield,
            name: qsTr('Heavy Shield'),
            category: StandardWeaponCategories.Martial
        });

        Weapons.register({
            id: StandardWeapons.HeavySpikedShield,
            name: qsTr('Heavy Spiked Shield'),
            category: StandardWeaponCategories.Martial
        });

        Weapons.register({
            id: StandardWeapons.Trident,
            name: qsTr('Trident'),
            category: StandardWeaponCategories.Martial
        });

        Weapons.register({
            id: StandardWeapons.Warhammer,
            name: qsTr('Warhammer'),
            category: StandardWeaponCategories.Martial
        });

        Weapons.register({
            id: StandardWeapons.Falchion,
            name: qsTr('Falchion'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Glaive,
            name: qsTr('Glaive'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Greataxe,
            name: qsTr('Greataxe'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Greatclub,
            name: qsTr('Greatclub'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.HeavyFlail,
            name: qsTr('Heavy Flail'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Greatsword,
            name: qsTr('Greataxe'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true,
            prototype: 4010
        });

        Weapons.register({
            id: StandardWeapons.Guisarme,
            name: qsTr('Guisarme'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Halberd,
            name: qsTr('Halberd'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Lance,
            name: qsTr('Lance'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Ranseur,
            name: qsTr('Ranseur'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Scythe,
            name: qsTr('Scythe'),
            category: StandardWeaponCategories.Martial,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Longbow,
            name: qsTr('Longbow'),
            category: StandardWeaponCategories.Martial,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.CompositeLongbow,
            name: qsTr('Composite Longbow'),
            category: StandardWeaponCategories.Martial,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.Shortbow,
            name: qsTr('Shortbow'),
            category: StandardWeaponCategories.Martial,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.CompositeShortbow,
            name: qsTr('Composite Shortbow'),
            category: StandardWeaponCategories.Martial,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.Kama,
            name: qsTr('Kama'),
            category: StandardWeaponCategories.Exotic,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.Nunchaku,
            name: qsTr('Nunchaku'),
            category: StandardWeaponCategories.Exotic,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.Sai,
            name: qsTr('Sai'),
            category: StandardWeaponCategories.Exotic,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.Siangham,
            name: qsTr('Siangham'),
            category: StandardWeaponCategories.Exotic,
            light: true
        });

        Weapons.register({
            id: StandardWeapons.BastardSword,
            name: 'Bastard Sword',
            category: StandardWeaponCategories.Exotic,
            prototype: 4015
        });

        Weapons.register({
            id: StandardWeapons.DwarvenWaraxe,
            name: qsTr('Dwarven Waraxe'),
            category: StandardWeaponCategories.Exotic
        });

        Weapons.register({
            id: StandardWeapons.Whip,
            name: qsTr('Whip'),
            category: StandardWeaponCategories.Exotic
        });

        Weapons.register({
            id: StandardWeapons.OrcDoubleAxe,
            name: qsTr('Orc Double Axe'),
            category: StandardWeaponCategories.Exotic,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.SpikedChain,
            name: qsTr('Spiked Chain'),
            category: StandardWeaponCategories.Exotic,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.DireFlail,
            name: qsTr('Dire Flail'),
            category: StandardWeaponCategories.Exotic,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.HookedGnomeHammer,
            name: qsTr('Hooked Gnome Hammer'),
            category: StandardWeaponCategories.Exotic,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.TwoBladedSword,
            name: qsTr('Two-bladed Sword'),
            category: StandardWeaponCategories.Exotic,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.DwarvenUrgrosh,
            name: qsTr('Dwarven Urgrosh'),
            category: StandardWeaponCategories.Exotic,
            twoHanded: true
        });

        Weapons.register({
            id: StandardWeapons.Bolas,
            name: qsTr('Bolas'),
            category: StandardWeaponCategories.Exotic,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.HandCrossbow,
            name: qsTr('Hand Crossbow'),
            category: StandardWeaponCategories.Exotic,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.HeavyRepeatingCrossbow,
            name: qsTr('Heavy Repeating Crossbow'),
            category: StandardWeaponCategories.Exotic,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.LightRepeatingCrossbow,
            name: qsTr('Light Repeating Crossbow'),
            category: StandardWeaponCategories.Exotic,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.Net,
            name: qsTr('Net'),
            category: StandardWeaponCategories.Exotic,
            ranged: true
        });

        Weapons.register({
            id: StandardWeapons.Shuriken,
            name: qsTr('Shuriken'),
            category: StandardWeaponCategories.Exotic,
            ranged: true
        });
    }

    StartupListeners.add(registerWeapons, 'd20-weapons');

})();
