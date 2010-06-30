
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

function handleDebugEvent(name) {
    if (name == 'loadMap') {
        showLoadMapWindow();
    } else if (name == 'spawnParticleSystem') {
        showSpawnParticleSystemWindow();
    }
}

function showDebugBar() {
    var debugBar = gameView.addGuiItem('interface/DebugBar.qml');
    debugBar.debugEvent.connect(handleDebugEvent);
}
