var loadMapUi = null;

function showLoadMapWindow() {
    if (loadMapUi != null)
        return;

    loadMapUi = gameView.addGuiItem('interface/LoadMap.qml');
    loadMapUi.setMapList(maps);
    loadMapUi.mapSelected.connect(function(dir) {
        loadMap('maps/' + dir + '/map.js');
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
