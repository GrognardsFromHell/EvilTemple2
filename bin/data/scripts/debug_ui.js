var loadMapUi = null;

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
    } else if (name == 'navmeshOverlay') {
        gameView.sectorMap.createDebugView();        
    }
}

var debugBar;

function showDebugBar() {
    debugBar = gameView.addGuiItem('interface/DebugBar.qml');
    debugBar.debugEvent.connect(handleDebugEvent);
}

// TODO: Replace this with a global gameStarted event
StartupListeners.add(function() {
    SaveGames.addLoadedListener(function() {
        showDebugBar();
    });

    Shortcuts.register(Keys.F10, function() {
        if (debugBar) {
            debugBar.takeScreenshot();
        } else {
            var screenshot = gameView.takeScreenshot();
            print("Taken screenshot " + screenshot);
        }
    });    
});
