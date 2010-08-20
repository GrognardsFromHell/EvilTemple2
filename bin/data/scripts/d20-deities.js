/**
 * Defines constants for the ids of deities defined in the d20 srd.
 */
var StandardDeities = {
    Boccob: 'boccob',
    CorellonLarethian: 'corellon_larethian',
    Ehlonna: 'ehlonna',
    Erythnul: 'erythnul',
    Fharlanghn: 'fharlanghn',
    GarlGlittergold: 'garl_glittergold',
    Gruumsh: 'gruumsh',
    Heironeous: 'heironeous',
    Hextor: 'hextor',
    Kord: 'kord',
    Moradin: 'moradin',
    Nerull: 'nerull',
    ObadHai: 'obad_hai',
    Olidammara: 'olidammara',
    Pelor: 'pelor',
    StCuthbert: 'st_cuthbert',
    Vecna: 'vecna',
    WeeJas: 'wee_jas',
    Yondalla: 'yondalla'
};

(function() {

    function registerDeities() {
        /*
         Register the greyhawk deities from ToEE.
         */
        Deities.register({
            id: StandardDeities.Boccob,
            name: translations.get('mes/deity/1'),
            description: translations.get('mes/deity/1001'),
            alignment: Alignment.TrueNeutral,
            domains: [StandardDomains.Knowledge, StandardDomains.Magic, StandardDomains.Trickery]
        });
        Deities.register({
            id: StandardDeities.CorellonLarethian,
            name: translations.get('mes/deity/2'),
            description: translations.get('mes/deity/1002'),
            alignment: Alignment.ChaoticGood,
            domains: [StandardDomains.Chaos, StandardDomains.Good, StandardDomains.Protection, StandardDomains.War]
        });
        Deities.register({
            id: StandardDeities.Ehlonna,
            name: translations.get('mes/deity/3'),
            description: translations.get('mes/deity/1003'),
            alignment: Alignment.NeutralGood,
            domains: [StandardDomains.Animal, StandardDomains.Good, StandardDomains.Plant, StandardDomains.Sun]
        });
        Deities.register({
            id: StandardDeities.Erythnul,
            name: translations.get('mes/deity/4'),
            description: translations.get('mes/deity/1004'),
            alignment: Alignment.ChaoticEvil,
            domains: [StandardDomains.Chaos, StandardDomains.Evil, StandardDomains.Trickery, StandardDomains.War]
        });
        Deities.register({
            id: StandardDeities.Fharlanghn,
            name: translations.get('mes/deity/5'),
            description: translations.get('mes/deity/1005'),
            alignment: Alignment.TrueNeutral,
            domains: [StandardDomains.Luck, StandardDomains.Protection, StandardDomains.Travel]
        });
        Deities.register({
            id: StandardDeities.GarlGlittergold,
            name: translations.get('mes/deity/6'),
            description: translations.get('mes/deity/1006'),
            alignment: Alignment.NeutralGood,
            domains: [StandardDomains.Good, StandardDomains.Protection, StandardDomains.Trickery]
        });
        Deities.register({
            id: StandardDeities.Gruumsh,
            name: translations.get('mes/deity/7'),
            description: translations.get('mes/deity/1007'),
            alignment: Alignment.ChaoticEvil,
            domains: [StandardDomains.Chaos, StandardDomains.Evil, StandardDomains.Strength, StandardDomains.War]
        });
        Deities.register({
            id: StandardDeities.Heironeous,
            name: translations.get('mes/deity/8'),
            description: translations.get('mes/deity/1008'),
            alignment: Alignment.LawfulGood,
            domains: [StandardDomains.Good, StandardDomains.Law, StandardDomains.War]
        });
        Deities.register({
            id: StandardDeities.Hextor,
            name: translations.get('mes/deity/9'),
            description: translations.get('mes/deity/1009'),
            alignment: Alignment.LawfulEvil,
            domains: [StandardDomains.Destruction, StandardDomains.Evil, StandardDomains.Law, StandardDomains.War]
        });
        Deities.register({
            id: StandardDeities.Kord,
            name: translations.get('mes/deity/10'),
            description: translations.get('mes/deity/1010'),
            alignment: Alignment.ChaoticGood,
            domains: [StandardDomains.Chaos, StandardDomains.Good, StandardDomains.Luck, StandardDomains.Strength]
        });
        Deities.register({
            id: StandardDeities.Moradin,
            name: translations.get('mes/deity/11'),
            description: translations.get('mes/deity/1011'),
            alignment: Alignment.LawfulGood,
            domains: [StandardDomains.Earth, StandardDomains.Good, StandardDomains.Law, StandardDomains.Protection]
        });
        Deities.register({
            id: StandardDeities.Nerull,
            name: translations.get('mes/deity/12'),
            description: translations.get('mes/deity/1012'),
            alignment: Alignment.NeutralEvil,
            domains: [StandardDomains.Death, StandardDomains.Evil, StandardDomains.Trickery]
        });
        Deities.register({
            id: StandardDeities.ObadHai,
            name: translations.get('mes/deity/13'),
            description: translations.get('mes/deity/1013'),
            alignment: Alignment.TrueNeutral,
            domains: [StandardDomains.Air, StandardDomains.Animal, StandardDomains.Earth, StandardDomains.Fire,
                StandardDomains.Plant, StandardDomains.Water]
        });
        Deities.register({
            id: StandardDeities.Olidammara,
            name: translations.get('mes/deity/14'),
            description: translations.get('mes/deity/1014'),
            alignment: Alignment.ChaoticNeutral,
            domains: [StandardDomains.Chaos, StandardDomains.Luck, StandardDomains.Trickery]
        });
        Deities.register({
            id: StandardDeities.Pelor,
            name: translations.get('mes/deity/15'),
            description: translations.get('mes/deity/1015'),
            alignment: Alignment.NeutralGood,
            domains: [StandardDomains.Good, StandardDomains.Healing, StandardDomains.Strength, StandardDomains.Sun]
        });
        Deities.register({
            id: StandardDeities.StCuthbert,
            name: translations.get('mes/deity/16'),
            description: translations.get('mes/deity/1016'),
            alignment: Alignment.LawfulNeutral,
            domains: [StandardDomains.Destruction, StandardDomains.Law,
                StandardDomains.Protection, StandardDomains.Strength]
        });
        Deities.register({
            id: StandardDeities.Vecna,
            name: translations.get('mes/deity/17'),
            description: translations.get('mes/deity/1017'),
            alignment: Alignment.NeutralEvil,
            domains: [StandardDomains.Evil, StandardDomains.Knowledge, StandardDomains.Magic]
        });
        Deities.register({
            id: StandardDeities.WeeJas,
            name: translations.get('mes/deity/18'),
            description: translations.get('mes/deity/1018'),
            alignment: Alignment.LawfulNeutral,
            domains: [StandardDomains.Death, StandardDomains.Law, StandardDomains.Magic]
        });
        Deities.register({
            id: StandardDeities.Yondalla,
            name: translations.get('mes/deity/19'),
            description: translations.get('mes/deity/1019'),
            alignment: Alignment.LawfulGood,
            domains: [StandardDomains.Good, StandardDomains.Law, StandardDomains.Protection]
        });
    }

    StartupListeners.add(registerDeities);

})();
