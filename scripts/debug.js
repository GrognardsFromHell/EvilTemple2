
/*
    This file contains debugging helper methods, that should *ONLY* be used from
    the console and NEVER be used from any other script.
 */

var Debug = {};

/**
 * Prints all available functions in the debug object.
 */
Debug.help = function() {
    var result = 'Available debug commands: \n';

    for (var k in Debug) {
        if (Debug.hasOwnProperty(k) && k != 'help')
            result += "Debug." + k + "\n";
    }

    return result;
};

Debug.showVisionExtenders = function() {
    Maps.currentMap.renderFog.revealAll();
    gameView.sectorMap.createDebugLayer("visionExtend", [1, 1, 1, 0.5]);
    return "Showing vision extenders & revealing map.";
};

Debug.showVisionSeparators = function() {
    Maps.currentMap.renderFog.revealAll();
    gameView.sectorMap.createDebugLayer("visionSeparator", [0, 1, 1, 0.5]);
    return "Showing vision separators & revealing map.";
};

Debug.showVisionBase = function() {
    Maps.currentMap.renderFog.revealAll();
    gameView.sectorMap.createDebugLayer("visionBase", [0, 0, 1, 0.5]);
    return "Showing vision base & revealing map.";
};

Debug.showVisionArchway = function() {
    Maps.currentMap.renderFog.revealAll();
    gameView.sectorMap.createDebugLayer("visionArchway", [0, 1, 0, 0.5]);
    return "Showing vision archways & revealing map.";
};

Debug.showVision = function() {
    Debug.showVisionExtenders();
    Debug.showVisionSeparators();
    Debug.showVisionBase();
    Debug.showVisionArchway();
    return "Showing all vision overlays & revealing map. (Gray: Extend, Cyan: End, Blue: Base, Green: Archway)";
};

/**
 * Makes all areas known to the player.
 */
Debug.markAllAreas = function() {
    WorldMap.mark("hommlet");
    WorldMap.mark("emridy_meadows");
    WorldMap.mark("moathouse");
    WorldMap.mark("moathouse_secret_exit");
    WorldMap.mark("nulb");
    WorldMap.mark("imeryds_run");
    WorldMap.mark("ogre_cave");
    WorldMap.mark("temple");
    WorldMap.mark("temple_secret_exit");
    WorldMap.mark("deklo_grove");
    return "Marked all worldmap areas.";
};
