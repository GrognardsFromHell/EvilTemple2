var TempleDeities = {
    OldFaith: 'old_faith',
    Zuggtmoy: 'zuggtmoy',
    Iuz: 'iuz',
    Lolth: 'lolth'
};

(function() {

    function registerDeities() {
        Deities.register({
            id: TempleDeities.OldFaith,
            name: translations.get('mes/deity/20'),
            description: translations.get('mes/deity/1020'),
            alignment: Alignment.TrueNeutral,
            // Uses the same domain list as obad-hai here, since the "deity" doesn't seem to have its own list.
            domains: [StandardDomains.Air, StandardDomains.Animal, StandardDomains.Earth, StandardDomains.Fire,
                StandardDomains.Plant, StandardDomains.Water]
        });
        Deities.register({
            id: TempleDeities.Zuggtmoy,
            name: translations.get('mes/deity/21'),
            description: translations.get('mes/deity/1021'),
            alignment: Alignment.ChaoticEvil,
            // Missing: Ooze
            domains: [StandardDomains.Chaos, StandardDomains.Evil, StandardDomains.Plant]
        });
        Deities.register({
            id: TempleDeities.Iuz,
            name: translations.get('mes/deity/22'),
            description: translations.get('mes/deity/1022'),
            alignment: Alignment.ChaoticEvil,
            // Missing: Mind, Suffering, Tyranny
            domains: [StandardDomains.Chaos, StandardDomains.Evil, StandardDomains.Trickery]
        });
        Deities.register({
            id: TempleDeities.Lolth,
            name: translations.get('mes/deity/23'),
            description: translations.get('mes/deity/1023'),
            alignment: Alignment.ChaoticEvil,
            // Domains may have changed, these are from the Forgotten Realms Wiki
            domains: [StandardDomains.Chaos, StandardDomains.Destruction, StandardDomains.Evil]
        });
    }

    StartupListeners.add(registerDeities);

})();
