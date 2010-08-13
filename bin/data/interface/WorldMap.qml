import Qt 4.7
import "WorldMapTravel.js" as WorldMapTravel
import "WorldMap.js" as WorldMap

MovableWindow {
    id: root
    width: 792
    height: 586
    title: "Worldmap"

    property variant knownAreas : ['hommlet']

    property string currentArea : ''

    signal travelRequested(string area)
    signal travelFinished

    property variant areas : [
        {
            'name': 'Moathouse',
            'area': 'moathouse',
            'center': [277, 454],
            'radius': 24,
            'images': [
                [-64, -64, "../art/interface/WORLDMAP_UI/WorldMap_Moathouse.png"],
                [-56, -109, "../art/interface/WORLDMAP_UI/WorldMap_Moathouse_Script.png"]
            ]
        },
        {
            'name': 'Emridy Meadows',
            'area': 'emridy_meadows',
            'center': [125, 167],
            'radius': 32,
            'images': [
                [-32, -32, "../art/interface/WORLDMAP_UI/WorldMap_Meadows.png"],
                [-91, -12, "../art/interface/WORLDMAP_UI/WorldMap_Meadows_Script.png"]
            ]
        },
        {
            'name': 'Moat House Cave Exit',
            'area': 'moathouse_secret_exit',
            'center': [318, 470],
            'radius': 16,
            'images': [
                [-32, -32, "../art/interface/WORLDMAP_UI/WorldMap_MoathouseCave.png"],
                [-8, -3, "../art/interface/WORLDMAP_UI/WorldMap_MoathouseCave_Script.png"]
            ]
        },
        {
            'name': 'Nulb',
            'area': 'nulb',
            'center': [417, 161],
            'radius': 20,
            'images': [
                [-64, -64, "../art/interface/WORLDMAP_UI/WorldMap_Nulb.png"],
                [-104, -117, "../art/interface/WORLDMAP_UI/WorldMap_Nulb_Script.png"]
            ]
        },
        {
            'name': 'Imeryds Run',
            'area': 'imeryds_run',
            'center': [435, 106],
            'radius': 20,
            'images': [
                [-32, -32, "../art/interface/WORLDMAP_UI/WorldMap_NulbRiverPool.png"],
                [-39, -45, "../art/interface/WORLDMAP_UI/WorldMap_NulbRiverPool_Script.png"]
            ]
        },
        {
            'name': 'Ogre Cave',
            'area': 'ogre_cave',
            'center': [251, 175],
            'radius': 30,
            'images': [
                [-64, -64, "../art/interface/WORLDMAP_UI/WorldMap_OgreCave.png"],
                [-57, -71, "../art/interface/WORLDMAP_UI/WorldMap_OgreCave_Script.png"]
            ]
        },
        {
            'name': 'Temple of Elemental Evil',
            'area': 'temple',
            'center': [436, 237],
            'radius': 16,
            'images': [
                [-42, -42, "../art/interface/WORLDMAP_UI/WorldMap_Temple.png"],
                [-119, -12, "../art/interface/WORLDMAP_UI/WorldMap_Temple_Script.png"]
            ]
        },
        {
            'name': 'Burnt Farmhouse',
            'area': 'temple_secret_exit',
            'center': [447, 197],
            'radius': 14,
            'images': [
                [-32, -32, "../art/interface/WORLDMAP_UI/WorldMap_TempleWell.png"],
                [-13, -46, "../art/interface/WORLDMAP_UI/WorldMap_TempleWell_Script.png"]
            ]
        },
        {
            'name': 'Deklo Grove',
            'area': 'deklo_grove',
            'center': [136, 338],
            'radius': 20,
            'images': [
                [-32, -32, "../art/interface/WORLDMAP_UI/WorldMap_DekloTrees.png"],
                [-49, -63, "../art/interface/WORLDMAP_UI/WorldMap_DekloTrees_Script.png"]
            ]
        },
        {
            'name': 'Temple Ruined House',
            'area': 'temple_ruined_house',
            'center': [408, 206],
            'radius': 14,
            'images': [
                [-25, -25, "../art/interface/WORLDMAP_UI/WorldMap_TempleHouse.png"],
            ]
        },
        {
            'name': 'Temple Broken Tower',
            'area': 'temple_tower',
            'center': [464, 241],
            'radius': 14,
            'images': [
                [-25, -25, "../art/interface/WORLDMAP_UI/WorldMap_TempleTower.png"],
            ]
        },
        {
            'name': 'Hommlet - South',
            'area': 'hommlet-south',
            'center': [133, 474],
            'radius': 16,
            'images': []
        },
        {
            'name': 'Hommlet - North',
            'area': 'hommlet-north',
            'center': [121, 439],
            'radius': 16,
            'images': []
        },
        {
            'name': 'Hommlet - East',
            'area': 'hommlet-east',
            'center': [179, 464],
            'radius': 16,
            'images': []
        }
    ]

    onAreasChanged: WorldMap.setAreas(areas)

    onCurrentAreaChanged: {
        console.debug("Updating you are here arrow for area: " + currentArea)

        youAreHere.visible = true;

        var centerOn;

        switch (currentArea) {
        case 'hommlet':
            youAreHere.x = 150 - 25;
            youAreHere.y = 420 - 25;
            return;
        case 'moathouse':
            // centerOn = moathouseButton;
            break;
        case 'moathouse_secret_exit':
            centerOn = moathouseCaveButton;
            break;
        case 'emridy_meadows':
            centerOn = meadowsButton;
            break;
        default:
            youAreHere.visible = false;
            return;
        }

        console.debug("X: " + centerOn.x)
        console.debug("Y: " + centerOn.y)

        youAreHere.x = centerOn.x + centerOn.width / 2 - youAreHere.width / 2;
        youAreHere.y = centerOn.y + centerOn.height / 2 - youAreHere.height / 2;

        console.debug("X: " + youAreHere.x)
        console.debug("Y: " + youAreHere.y)
    }

    function distance(x1, y1, x2, y2) {
        var dx = x1 - x2;
        var dy = y1 - y2;
        return Math.sqrt(dx * dx + dy * dy);
    }

    /**
        Travels along a path on the worldmap.
        The given object is expected to have the following properties:
        from: [x,y]
        to: [x,y]
        path: [e,e,e,e,...]
      */
    function travelPath(path) {
        WorldMapTravel.startTravel(path);
    }

    Timer {
        id: travelTimer
        interval: 20
        repeat: true
        onTriggered: WorldMapTravel.doTravelStep()
    }

    function refreshHighlight(mouseX, mouseY) {
        var centerX, centerY, maxRadius;
        var worldmapButton;

        for (var i = 0; i < children.length; ++i) {
            var child = root.children[i];

            maxRadius = child.maxRadius;

            if (!maxRadius || !child.visible)
                continue;

            centerX = (child.x + child.width / 2);
            centerY = (child.y + child.height / 2);

            var diffX = mouseX - centerX;
            var diffY = mouseY - centerY;
            var distance = Math.sqrt(diffX * diffX + diffY * diffY);

            if (distance <= maxRadius) {
                highlightOverlay.x = centerX - maxRadius;
                highlightOverlay.y = centerY - maxRadius;
                highlightOverlay.width = maxRadius * 2;
                highlightOverlay.height = maxRadius * 2;
                worldmapButton = child;
                break;
            }
        }

        if (worldmapButton) {
            highlightOverlay.state = 'visible';
            tooltip.text = worldmapButton.name;
            tooltip.x = centerX - tooltip.width / 2;
            tooltip.y = centerY + maxRadius;
            tooltip.shown = true;
        } else {
            highlightOverlay.state = '';
            tooltip.shown = false;
        }
    }

    Image {
        anchors.fill: parent
        source: "../art/interface/WORLDMAP_UI/worldmap-main.png"
    }

    Button {
        x: 730
        y: 525
        width: 53
        height: 50
        disabledImage: "art/interface/CHAR_UI/main_exit_button_disabled.png"
        pressedImage: "art/interface/CHAR_UI/main_exit_button_pressed.png"
        normalImage: "art/interface/CHAR_UI/main_exit_button_hover_off.png"
        hoverImage: "art/interface/CHAR_UI/main_exit_button_hover_on.png"
        text: ''
        onClicked: root.closeClicked()
    }

    Image {
        id: highlightOverlay
        x: 383
        y: 373
        width: 100
        height: 100
        opacity: 0
        source: "../art/interface/WORLDMAP_UI/Worldmap_Ring.png"
        z: 100

        states: [
            State {
                name: "visible"
                PropertyChanges {
                    target: highlightOverlay
                    opacity: 0.75
                }
            }
        ]

        transitions: [
            Transition {
                from: "*"
                to: "*"
                NumberAnimation { property: "opacity"; duration: 200 }
            }
        ]

    }

    MouseArea {
        id: worldMapMouseArea
        x: 42
        y: 37
        width: 466
        height: 506
        hoverEnabled: true

        onMousePositionChanged: {
            var pos = mapToItem(root, mouseX, mouseY);
            root.refreshHighlight(pos.x, pos.y);
        }

        onClicked: {
            var pos = mapToItem(root, mouseX, mouseY);
            WorldMap.onClick(pos.x, pos.y);
        }
    }

    Image {
        id: youAreHere
        x: 133
        y: 391
        width: 50
        height: 50
        smooth: true
        source: "../art/interface/WORLDMAP_UI/Worldmap_You_are_here.png"
    }

    Tooltip {
        id: tooltip
    }

    ListModel {
        id: knownAreaModel
    }

    ListView {
        id: knownAreaList
        x: 572
        y: 99
        width: 186
        height: 375
        model: knownAreaModel
        clip: true
        delegate: Text {
            color: mouseArea.containsMouse ? '#00a9f8' : '#FFFFFF'
            text: name
            font.family: 'Fontin'
            font.pointSize: 12
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    console.log("Travel requested to: " + area);
                    root.travelRequested(area);
                }
            }
        }
        boundsBehavior: "StopAtBounds"
    }

}
