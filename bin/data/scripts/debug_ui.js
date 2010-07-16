var loadMapUi = null;

/*var maps = [
    { name: 'Temple Level 1', dir: 'Map12-temple-dungeon-level-1' },
    { name: "Temple Level 1 (Room)", dir: 'Map12-temple-dungeon-level-1-room-131' },
    { name: "Temple Level 2", dir: "Map13-dungeon-level-02" },
    { name: "Temple Level 3 (Lower)", dir: "Map14-dungeon-level-03_lower" },
    { name: "Temple Level 3 (Upper)", dir: "Map14-dungeon-level-03_upper" },
    { name: "Temple Level 4", dir: "Map15-dungeon-level-04" },
    { name: 'Hommlet', dir: 'Map-2-Hommlet-Exterior' },
    { name: 'Moathouse Interior', dir: 'Map-7-Moathouse_Interior' },
    { name: 'Nulb', dir: 'Map-9-Nulb-Exterior' },
    { name: 'Air Node', dir: 'Map16-air-node' },
    { name: 'Fire Node', dir: 'Map18-fire-node' },
    { name: 'Water Node', dir: 'Map19-water-node' },
    { name: 'Colosseum', dir: 'Map-49-Colosseum' },
    { name: 'Emridy Meadows', dir: 'Map-Area-5-Emridy-Meadows' },
    { name: 'Imeryds Run', dir: 'Map-Area-6-Imeryds-Run' },
    { name: 'Ogre Cave', dir: 'Map-Area-9-Ogre-Cave-Exterior' },
    { name: 'Decklo Grove', dir: 'Map-Area-10-Decklo-Grove' },
    { name: 'Tutorial Map', dir: 'Tutorial Map 1' },
    { name: 'Vignette Lawful Good', dir: 'Vignette-Lawful-Good' },
    { name: 'Vignette Good', dir: 'Vignette-Good' },
    { name: 'Vignette Chaotic-Good', dir: 'Vignette-Chaotic-Good' },
    { name: 'Vignette Lawful', dir: 'Vignette-Lawful' },
    { name: 'Vignette Neutral', dir: 'Vignette-Neutral' },
    { name: 'Vignette Chaotic', dir: 'Vignette-Chaotic' },
    { name: 'Vignette Lawful Evil', dir: 'Vignette-Lawful-Evil' },
    { name: 'Vignette Evil', dir: 'Vignette-Evil' },
    { name: 'Vignette Chaotic Evil', dir: 'Vignette-Chaotic-Evil' },
    { name: 'Random Riverside Road', dir: 'Random-Riverside-Road' }
];*/

function showLoadMapWindow() {
    if (loadMapUi != null)
        return;

    var mapList = [];
    Maps.maps.forEach(function (map) {
        mapList.push({
            name: map.name + ' (' + map.legacyId + ')',
            dir: map.id
        });
    });

    loadMapUi = gameView.addGuiItem('interface/LoadMap.qml');
    loadMapUi.mapList = mapList;
    loadMapUi.mapSelected.connect(function(mapId) {
        var newMap = Maps.mapsById[mapId];
        if (!newMap) {
            print("Unable to find map id: " + mapId);
        } else {
            print("STARTLOC: "+ newMap.startPosition);
            Maps.goToMap(newMap, newMap.startPosition);
        }
    });
    loadMapUi.closeClicked.connect(function() {
        loadMapUi.deleteLater();
        loadMapUi = null;
    });
}

var spawnPartSysUi = null;

function showSpawnParticleSystemWindow() {
    if (spawnPartSysUi != null)
        return;

    spawnPartSysUi = gameView.addGuiItem('interface/SpawnParticleSystem.qml');
    spawnPartSysUi.spawnParticleSystem.connect(function(name) {
        selectWorldTarget(function(position) {
            var sceneNode = gameView.scene.createNode();
            sceneNode.position = position;

            var particleSystem = gameView.particleSystems.instantiate(name);
            sceneNode.attachObject(particleSystem);
        });
    });
    spawnPartSysUi.closeClicked.connect(function() {
        spawnPartSysUi.deleteLater();
        spawnPartSysUi = null;
    });
}

var consoleWindow = null;

function showConsoleWindow() {
    if (consoleWindow != null)
        return;

    consoleWindow = gameView.addGuiItem('interface/Console.qml');

    consoleWindow.commandIssued.connect(function(command) {
        try {
            consoleWindow.appendResult(eval(command));
        } catch (e) {
            consoleWindow.appendResult('<b style="color: red">ERROR:</b> ' + e);
        }
    });

    consoleWindow.closeClicked.connect(function() {
        consoleWindow.deleteLater();
        consoleWindow = null;
    });
}

function handleDebugEvent(name) {
    if (name == 'loadMap') {
        showLoadMapWindow();
    } else if (name == 'spawnParticleSystem') {
        showSpawnParticleSystemWindow();
    } else if (name == 'openConsole') {
        showConsoleWindow();
    }
}

function showDebugBar() {
    var debugBar = gameView.addGuiItem('interface/DebugBar.qml');
    debugBar.debugEvent.connect(handleDebugEvent);

    Shortcuts.register(Keys.F10, function() {
        debugBar.takeScreenshot();
    });
}
