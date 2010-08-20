var Deities = {};

(function() {

    var byId = {};
    var list = [];

    Deities.register = function(deity) {
        byId[deity.id] = deity;
        list.push(deity);
    };

    Deities.getAll = function() {
        print("Getting " + list.length + " deities.");
        return list.slice(0);
    };

    Deities.getById = function(id) {
        return byId[id];
    };

    function registerDeities() {
        /*
         Register the greyhawk deities from ToEE.
         */
        Deities.register({
            id: 'boccob',
            name: translations.get('mes/deity/1'),
            description: translations.get('mes/deity/1001'),
            alignment: Alignment.TrueNeutral
        });
        Deities.register({
            id: 'corellon_larethian',
            name: translations.get('mes/deity/2'),
            description: translations.get('mes/deity/1002'),
            alignment: Alignment.ChaoticGood
        });
        Deities.register({
            id: 'ehlonna',
            name: translations.get('mes/deity/3'),
            description: translations.get('mes/deity/1003'),
            alignment: Alignment.NeutralGood
        });
        Deities.register({
            id: 'erythnul',
            name: translations.get('mes/deity/4'),
            description: translations.get('mes/deity/1004'),
            alignment: Alignment.ChaoticEvil
        });
        Deities.register({
            id: 'fharlanghn',
            name: translations.get('mes/deity/5'),
            description: translations.get('mes/deity/1005'),
            alignment: Alignment.Neutral
        });
        Deities.register({
            id: 'garl_glittergold',
            name: translations.get('mes/deity/6'),
            description: translations.get('mes/deity/1006'),
            alignment: Alignment.NeutralGood
        });
        Deities.register({
            id: 'gruumsh',
            name: translations.get('mes/deity/7'),
            description: translations.get('mes/deity/1007'),
            alignment: Alignment.ChaoticEvil
        });
        Deities.register({
            id: 'heironeous',
            name: translations.get('mes/deity/8'),
            description: translations.get('mes/deity/1008'),
            alignment: Alignment.LawfulGood
        });
        Deities.register({
            id: 'hextor',
            name: translations.get('mes/deity/9'),
            description: translations.get('mes/deity/1009'),
            alignment: Alignment.LawfulEvil
        });
        Deities.register({
            id: 'kord',
            name: translations.get('mes/deity/10'),
            description: translations.get('mes/deity/1010'),
            alignment: Alignment.ChaoticGood
        });
        Deities.register({
            id: 'moradin',
            name: translations.get('mes/deity/11'),
            description: translations.get('mes/deity/1011'),
            alignment: Alignment.LawfulGood
        });
        Deities.register({
            id: 'nerull',
            name: translations.get('mes/deity/12'),
            description: translations.get('mes/deity/1012'),
            alignment: Alignment.NeutralEvil
        });
        Deities.register({
            id: 'obad_hai',
            name: translations.get('mes/deity/13'),
            description: translations.get('mes/deity/1013'),
            alignment: Alignment.TrueNeutral
        });
        Deities.register({
            id: 'olidammara',
            name: translations.get('mes/deity/14'),
            description: translations.get('mes/deity/1014'),
            alignment: Alignment.ChaoticNeutral
        });
        Deities.register({
            id: 'pelor',
            name: translations.get('mes/deity/15'),
            description: translations.get('mes/deity/1015'),
            alignment: Alignment.NeutralGood
        });
        Deities.register({
            id: 'st_cuthbert',
            name: translations.get('mes/deity/16'),
            description: translations.get('mes/deity/1016'),
            alignment: Alignment.LawfulNeutral
        });
        Deities.register({
            id: 'vecna',
            name: translations.get('mes/deity/17'),
            description: translations.get('mes/deity/1017'),
            alignment: Alignment.NeutralEvil
        });
        Deities.register({
            id: 'wee_jas',
            name: translations.get('mes/deity/18'),
            description: translations.get('mes/deity/1018'),
            alignment: Alignment.LawfulNeutral
        });
        Deities.register({
            id: 'yondalla',
            name: translations.get('mes/deity/19'),
            description: translations.get('mes/deity/1019'),
            alignment: Alignment.LawfulGood
        });
        Deities.register({
            id: 'old_faith',
            name: translations.get('mes/deity/20'),
            description: translations.get('mes/deity/1020'),
            alignment: Alignment.TrueNeutral
        });
        Deities.register({
            id: 'zuggtmoy',
            name: translations.get('mes/deity/21'),
            description: translations.get('mes/deity/1021'),
            alignment: Alignment.ChaoticEvil
        });
        Deities.register({
            id: 'iuz',
            name: translations.get('mes/deity/22'),
            description: translations.get('mes/deity/1022'),
            alignment: Alignment.ChaoticEvil
        });
        Deities.register({
            id: 'lolth',
            name: translations.get('mes/deity/23'),
            description: translations.get('mes/deity/1023'),
            alignment: Alignment.ChaoticEvil
        });
    }

    StartupListeners.add(registerDeities);

})();
