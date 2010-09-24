var WorldMapUi = {};

(function() {

    /**
     * The possible worldmap buttons.
     */
    var worldmapButtons = {
        'emridy_meadows': {
            'name': 'Emridy Meadows',
            'area': 'emridy_meadows',
            'destinationMap': 'map-area-5-emridy-meadows',
            'center': [123, 174],
            'radius': 30,
            'images': [
                [-31, -29, "../art/interface/WORLDMAP_UI/WorldMap_Meadows.png"],
                [-80, -9, "../art/interface/WORLDMAP_UI/WorldMap_Meadows_Script.png"]
            ]
        },
        'moathouse': {
            'name': 'Moathouse',
            'area': 'moathouse',
            'destinationMap': 'map-7-ruins-of-the-moathouse',
            'center': [282, 462],
            'radius': 24,
            'images': [
                [-60, -62, "../art/interface/WORLDMAP_UI/WorldMap_Moathouse.png"],
                [-52, -107, "../art/interface/WORLDMAP_UI/WorldMap_Moathouse_Script.png"]
            ]
        },
        'moathouse_secret_exit': {
            'name': 'Moat House Cave Exit',
            'area': 'moathouse_secret_exit',
            'destinationMap': 'moathouse-exit-tunnel',
            'center': [326, 481],
            'radius': 16,
            'images': [
                [-31, -33, "../art/interface/WORLDMAP_UI/WorldMap_MoathouseCave.png"],
                [-7, -4, "../art/interface/WORLDMAP_UI/WorldMap_MoathouseCave_Script.png"]
            ]
        },
        'nulb': {
            'name': 'Nulb',
            'area': 'nulb',
            'destinationMap': 'map-9-nulb-exterior',
            'center': [427, 170],
            'radius': 20,
            'images': [
                [-65, -63, "../art/interface/WORLDMAP_UI/WorldMap_Nulb.png"],
                [-105, -116, "../art/interface/WORLDMAP_UI/WorldMap_Nulb_Script.png"]
            ]
        },
        'imeryds_run': {
            'name': 'Imeryds Run',
            'area': 'imeryds_run',
            'destinationMap': 'map-area-6-imeryds-run',
            'center': [442, 116],
            'radius': 20,
            'images': [
                [-30, -32, "../art/interface/WORLDMAP_UI/WorldMap_NulbRiverPool.png"],
                [-37, -45, "../art/interface/WORLDMAP_UI/WorldMap_NulbRiverPool_Script.png"]
            ]
        },
        'ogre_cave': {
            'name': 'Ogre Cave',
            'area': 'ogre_cave',
            'destinationMap': 'map-area-9-ogre-cave-exterior',
            'center': [261, 184],
            'radius': 30,
            'images': [
                [-65, -63, "../art/interface/WORLDMAP_UI/WorldMap_OgreCave.png"],
                [-58, -70, "../art/interface/WORLDMAP_UI/WorldMap_OgreCave_Script.png"]
            ]
        },
        'temple': {
            'name': 'Temple of Elemental Evil',
            'area': 'temple',
            'destinationMap': 'map-40-temple-entrance',
            'center': [443, 248],
            'radius': 16,
            'images': [
                [-40, -43, "../art/interface/WORLDMAP_UI/WorldMap_Temple.png"],
                [-117, -13, "../art/interface/WORLDMAP_UI/WorldMap_Temple_Script.png"]
            ]
        },
        'temple_secret_exit': {
            'name': 'Burnt Farmhouse',
            'area': 'temple_secret_exit',
            'destinationMap': 'temple-deserted-farm',
            'center': [456, 206],
            'radius': 14,
            'images': [
                [-32, -31, "../art/interface/WORLDMAP_UI/WorldMap_TempleWell.png"],
                [-13, -45, "../art/interface/WORLDMAP_UI/WorldMap_TempleWell_Script.png"]
            ]
        },
        'deklo_grove': {
            'name': 'Deklo Grove',
            'area': 'deklo_grove',
            'destinationMap': 'map-area-10-decklo-grove',
            'center': [144, 350],
            'radius': 20,
            'images': [
                [-31, -34, "../art/interface/WORLDMAP_UI/WorldMap_DekloTrees.png"],
                [-48, -65, "../art/interface/WORLDMAP_UI/WorldMap_DekloTrees_Script.png"]
            ]
        },
        'temple_ruined_house': {
            'name': 'Temple Ruined House',
            'area': 'temple_ruined_house',
            'destinationMap': 'map-41-temple-house',
            'center': [415, 217],
            'radius': 14,
            'images': [
                [-23, -26, "../art/interface/WORLDMAP_UI/WorldMap_TempleHouse.png"]
            ]
        },
        'temple_tower': {
            'name': 'Temple Broken Tower',
            'area': 'temple_tower',
            'destinationMap': 'map-10-tower-ruins',
            'center': [473, 254],
            'radius': 14,
            'images': [
                [-25, -28, "../art/interface/WORLDMAP_UI/WorldMap_TempleTower.png"]
            ]
        },
        'hommlet-south': {
            'name': 'Hommlet - South',
            'area': 'hommlet',
            'destinationMap': 'map-2-hommlet-exterior',
            'destinationPosition': [17621, 0, 11907], // Overrides map's start pos
            'center': [133, 474],
            'radius': 16,
            'images': [],
            'youAreHere': [150, 420] // Special coordinates for the you-are-here arrow
        },
        'hommlet-north': {
            'name': 'Hommlet - North',
            'area': 'hommlet',
            'destinationMap': 'map-2-hommlet-exterior',
            'destinationPosition': [14481, 0, 6250], // Overrides map's start pos
            'center': [121, 439],
            'radius': 16,
            'images': [],
            'youAreHere': [150, 420] // Special coordinates for the you-are-here arrow
        },
        'hommlet-east': {
            'name': 'Hommlet - East',
            'area': 'hommlet',
            'destinationMap': 'map-2-hommlet-exterior',
            'destinationPosition': [12261, 0, 17720], // Overrides map's start pos            
            'center': [179, 464],
            'radius': 16,
            'images': [],
            'youAreHere': [150, 420] // Special coordinates for the you-are-here arrow
        }
    };

    /**
     * These define the connection that each path establishes.
     */
    var pathConnections = {
        0: ['hommlet', 'emridy_meadows'],
        1: ['hommlet', 'moathouse'],
        2: ['hommlet', 'moathouse_secret_exit'],
        3: ['hommlet', 'nulb'],
        4: ['hommlet', 'imeryds_run'],
        5: ['hommlet', 'ogre_cave'],
        6: ['hommlet', 'temple'],
        7: ['hommlet', 'temple_secret_exit'],
        8: ['hommlet', 'deklo_grove'],
        9: ['emridy_meadows', 'deklo_grove'],
        10: ['deklo_grove', 'moathouse'],
        11: ['deklo_grove', 'moathouse_secret_exit'],
        12: ['deklo_grove', 'nulb'],
        13: ['deklo_grove', 'imeryds_run'],
        14: ['deklo_grove', 'ogre_cave'],
        15: ['deklo_grove', 'temple'],
        16: ['deklo_grove', 'temple_secret_exit'],
        17: ['emridy_meadows', 'temple_secret_exit'],
        18: ['moathouse', 'temple_secret_exit'],
        19: ['moathouse_secret_exit', 'temple_secret_exit'],
        20: ['imeryds_run', 'temple_secret_exit'],
        21: ['nulb', 'temple_secret_exit'],
        22: ['ogre_cave', 'temple_secret_exit'],
        23: ['temple', 'temple_secret_exit'],
        24: ['emridy_meadows', 'temple'],
        25: ['moathouse', 'temple'],
        26: ['moathouse_secret_exit', 'temple'],
        27: ['ogre_cave', 'temple'],
        28: ['nulb', 'temple'],
        29: ['temple', 'imeryds_run'],
        30: ['emridy_meadows', 'ogre_cave'],
        31: ['ogre_cave', 'moathouse'],
        32: ['ogre_cave', 'moathouse_secret_exit'],
        33: ['ogre_cave', 'nulb'],
        34: ['ogre_cave', 'imeryds_run'],
        35: ['emridy_meadows', 'imeryds_run'],
        36: ['moathouse', 'imeryds_run'],
        37: ['moathouse_secret_exit', 'imeryds_run'],
        38: ['nulb', 'imeryds_run'],
        39: ['emridy_meadows', 'nulb'],
        40: ['moathouse', 'nulb'],
        41: ['moathouse_secret_exit', 'nulb'],
        42: ['emridy_meadows', 'moathouse_secret_exit'],
        43: ['moathouse', 'moathouse_secret_exit'],
        44: ['emridy_meadows', 'moathouse']
    };

    var worldMapPaths = null;

    var worldMapDialog = null;

    var newAreaSinceLastOpened = false;

    function loadPaths() {
        if (worldMapPaths == null) {
            worldMapPaths = readJson('worldmapPaths.js');
        }
    }

    var reversedOpcodes = {
        'up': 'down',
        'down': 'up',
        'left': 'right',
        'right': 'left',
        'upleft': 'downright',
        'upright': 'downleft',
        'downleft': 'upright',
        'downright': 'upleft',
        'stay': 'stay'
    };

    /**
     * Returns the inverse of a path opcode.
     * @param opcode The opcode to invert.
     */
    function invertPathOpcode(opcode) {
        var result = reversedOpcodes[opcode];
        if (!result) {
            print("Unknown opcode encountered while reversing path: " + opcode);
            result = opcode;
        }
        return result;
    }

    /**
     * Reverses a path if it is travelled in reverse.
     * @param path The path to reverse.
     */
    function invertPath(path) {
        var newPath = {
            'from': path.to,
            'to': path.from,
            'path': path.path.map(invertPathOpcode) // Shallow copy
        };

        newPath.path.reverse(); // Don't forget to reverse the actual order of the opcodes

        return newPath;
    }

    StartupListeners.add(function() {
        WorldMap.addMarkedNewAreaListener(function() {
            if (worldMapDialog) {
                // Play sound immediately & update ui
                worldMapDialog.knownAreas = WorldMap.getMarkedAreas();
                playNewAreaSound();
            } else {
                newAreaSinceLastOpened = true;
            }
        });
    });

    function travelTo(buttonId) {
        var button = worldmapButtons[buttonId];

        if (!button) {
            print("Unknown worldmap button clicked: " + buttonId);
            return;
        }

        if (!WorldMap.isMarked(button.area)) {
            print("Clicked on button " + buttonId + " for an unmarked area: " + button.area);
            return;
        }

        var map = Maps.mapsById[button.destinationMap];

        if (!map) {
            print("Button " + buttonId + " has unknown destination map: " + button.destinationMap);
            return;
        }

        var position = map.startPosition;
        if (button.destinationPosition)
            position = button.destinationPosition;

        Maps.goToMap(map, position);
    }

    WorldMapUi.show = function() {
        if (worldMapDialog)
            return;

        worldMapDialog = gameView.addGuiItem("interface/WorldMap.qml");
        worldMapDialog.closeClicked.connect(WorldMapUi.close);

        var knownAreas = [];
        WorldMap.getMarkedAreas().forEach(function (knownArea) {
            for (var buttonId in worldmapButtons) {
                var button = worldmapButtons[buttonId];
                if (button.area == knownArea) {
                    knownAreas.push({
                        'buttonId': buttonId,
                        'name': button.name,
                        'center': button.center,
                        'radius': button.radius,
                        'images': button.images
                    });
                }
            }
        });

        worldMapDialog.areas = knownAreas;

        // Update you are here arrow
        worldMapDialog.hideYouAreHere();
        for (var buttonId in worldmapButtons) {
            var button = worldmapButtons[buttonId];
            if (button.area == Maps.currentMap.area) {
                // Use this button's center x/y as the you are here center, if there's no override
                if (button.youAreHere)
                    worldMapDialog.showYouAreHere(button.youAreHere[0], button.youAreHere[1]);
                else
                    worldMapDialog.showYouAreHere(button.center[0], button.center[1]);
                break;
            }
        }

        if (newAreaSinceLastOpened) {
            playNewAreaSound();
            newAreaSinceLastOpened = false;
        }

        loadPaths();
        var travellingTo;
        worldMapDialog.travelRequested.connect(function (to) {
            print("Worldmap requested travel to " + to);
            var from = Maps.currentMap.area;

            travellingTo = to;

            if (!worldmapButtons[to]) {
                print("Clicked on unknown button: " + to);
                return;
            }

            // Path connections are between areas and not between worldmap buttons.
            var toArea = worldmapButtons[to].area;

            // Try to find a connection.
            for (var pathId in pathConnections) {
                var connection = pathConnections[pathId];

                if (connection[0] == from && connection[1] == toArea) {
                    worldMapDialog.travelPath(worldMapPaths[pathId]);
                    return;
                } else if (connection[1] == from && connection[0] == toArea) {
                    worldMapDialog.travelPath(invertPath(worldMapPaths[pathId]));
                    return;
                }
            }

            // No path could be found. Instant travel...
            print("No path found for connection " + from + " -> " + to);
            travelTo(travellingTo);
        });
        worldMapDialog.travelFinished.connect(function() {
            print("Travelling is finished. Arrived at new destination.");
            WorldMapUi.close();
            travelTo(travellingTo);
        });
    };

    WorldMapUi.close = function() {
        if (!worldMapDialog)
            return;

        worldMapDialog.deleteLater();
        worldMapDialog = null;
    };

    function playNewAreaSound() {
        gameView.playUiSound('sound/worldmap_location.wav');
    }

})();
