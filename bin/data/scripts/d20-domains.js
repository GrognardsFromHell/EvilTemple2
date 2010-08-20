/**
 * Defines identifiers for domains from the D20 SRD.
 */
var StandardDomains = {
    Air: 'air',
    Animal: 'animal',
    Chaos: 'chaos',
    Death: 'death',
    Destruction: 'destruction',
    Earth: 'earth',
    Evil: 'evil',
    Fire: 'fire',
    Good: 'good',
    Healing: 'healing',
    Knowledge: 'knowledge',
    Law: 'law',
    Luck: 'luck',
    Magic: 'magic',
    Plant: 'plant',
    Protection: 'protection',
    Strength: 'strength',
    Sun: 'sun',
    Travel: 'travel',
    Trickery: 'trickery',
    War: 'war',
    Water: 'water'
};

(function() {

    /*
     Register default domains.
     */
    function registerDomains() {

        Domains.register({
            id: StandardDomains.Air,
            name: translations.get('mes/spell/4001'),
            description: 'TAG_AIR_D'
        });

        Domains.register({
            id: StandardDomains.Animal,
            name: translations.get('mes/spell/4002'),
            description: 'TAG_ANIMAL_D'
        });

        Domains.register({
            id: StandardDomains.Chaos,
            name: translations.get('mes/spell/4003'),
            description: 'TAG_CHAOS_D'
        });

        Domains.register({
            id: StandardDomains.Death,
            name: translations.get('mes/spell/4004'),
            description: 'TAG_DEATH_D'
        });

        Domains.register({
            id: StandardDomains.Destruction,
            name: translations.get('mes/spell/4005'),
            description: 'TAG_DESTRUCTION_D'
        });

        Domains.register({
            id: StandardDomains.Earth,
            name: translations.get('mes/spell/4006'),
            description: 'TAG_EARTH_D'
        });

        Domains.register({
            id: StandardDomains.Evil,
            name: translations.get('mes/spell/4007'),
            description: 'TAG_EVIL_D'
        });

        Domains.register({
            id: StandardDomains.Fire,
            name: translations.get('mes/spell/4008'),
            description: 'TAG_FIRE_D'
        });

        Domains.register({
            id: StandardDomains.Good,
            name: translations.get('mes/spell/4009'),
            description: 'TAG_GOOD_DOMAIN'
        });

        Domains.register({
            id: StandardDomains.Healing,
            name: translations.get('mes/spell/4010'),
            description: 'TAG_HEALING_D'
        });

        Domains.register({
            id: StandardDomains.Knowledge,
            name: translations.get('mes/spell/4011'),
            description: 'TAG_KNOWLEDGE_D'
        });

        Domains.register({
            id: StandardDomains.Law,
            name: translations.get('mes/spell/4012'),
            description: 'TAG_LAW_D'
        });

        Domains.register({
            id: StandardDomains.Luck,
            name: translations.get('mes/spell/4013'),
            description: 'TAG_LUCK_D'
        });

        Domains.register({
            id: StandardDomains.Magic,
            name: translations.get('mes/spell/4014'),
            description: 'TAG_MAGIC_D'
        });

        Domains.register({
            id: StandardDomains.Plant,
            name: translations.get('mes/spell/4015'),
            description: 'TAG_PLANT_D'
        });

        Domains.register({
            id: StandardDomains.Protection,
            name: translations.get('mes/spell/4016'),
            description: 'TAG_PROTECTION_D'
        });

        Domains.register({
            id: StandardDomains.Strength,
            name: translations.get('mes/spell/4017'),
            description: 'TAG_STRENGTH_D'
        });

        Domains.register({
            id: StandardDomains.Sun,
            name: translations.get('mes/spell/4018'),
            description: 'TAG_SUN_D'
        });

        Domains.register({
            id: StandardDomains.Travel,
            name: translations.get('mes/spell/4019'),
            description: 'TAG_TRAVEL_D'
        });

        Domains.register({
            id: StandardDomains.Trickery,
            name: translations.get('mes/spell/4020'),
            description: 'TAG_TRICKERY_D'
        });

        Domains.register({
            id: StandardDomains.War,
            name: translations.get('mes/spell/4021'),
            description: 'TAG_WAR_D'
        });

        Domains.register({
            id: StandardDomains.Water,
            name: translations.get('mes/spell/4022'),
            description: 'TAG_WATER_D'
        });

    }

    StartupListeners.add(registerDomains);

})();
