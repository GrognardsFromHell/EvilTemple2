import Qt 4.7


MovableWindow {
    id: root
    width: 792
    height: 586
    title: "Worldmap"

    property variant knownAreas : ['hommlet']

    property string currentArea : ''

    onKnownAreasChanged: {
        console.debug("Updating visible areas.");

        if (knownAreas.indexOf('hommlet') != -1) {
            hommletSouth.visible = true;
            hommletEast.visible = true;
            hommletNorth.visible = true;
        } else {
            hommletSouth.visible = false;
            hommletEast.visible = false;
            hommletNorth.visible = false;
        }

        nulbButton.visible = knownAreas.indexOf('nulb') != -1;
        moathouseButton.visible = knownAreas.indexOf('moathouse') != -1;
        moathouseCaveButton.visible = knownAreas.indexOf('moathouse_secret_exit') != -1;
        meadowsButton.visible = knownAreas.indexOf('emridy_meadows') != -1;
        nulbRiverPoolButton.visible = knownAreas.indexOf('imeryds_run') != -1;
        templeWellButton.visible = knownAreas.indexOf('temple_secret_exit') != -1;
        ogreCaveButton.visible = knownAreas.indexOf('ogre_cave') != -1;
        dekloTreesButton.visible = knownAreas.indexOf('deklo_grove') != -1;
        templeHouseButton.visible = knownAreas.indexOf('temple_ruined_house') != -1;
        templeTowerButton.visible = knownAreas.indexOf('temple_tower') != -1;
        templeButton.visible = knownAreas.indexOf('temple') != -1;
    }

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
            centerOn = moathouseButton;
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

    function refreshHighlight(mouseX, mouseY) {
        var found = false;

        for (var i = 0; i < children.length; ++i) {
            var child = root.children[i];

            var maxRadius = child.maxRadius;

            if (!maxRadius || !child.visible)
                continue;

            var centerX = (child.x + child.width / 2);
            var centerY = (child.y + child.height / 2);

            var diffX = mouseX - centerX;
            var diffY = mouseY - centerY;
            var distance = Math.sqrt(diffX * diffX + diffY * diffY);

            if (distance <= maxRadius) {
                highlightOverlay.x = centerX - maxRadius;
                highlightOverlay.y = centerY - maxRadius;
                highlightOverlay.width = maxRadius * 2;
                highlightOverlay.height = maxRadius * 2;
                found = true;
                break;
            }
        }

        if (found)
            highlightOverlay.state = 'visible';
        else
            highlightOverlay.state = '';
    }

    Image {
        anchors.fill: parent
        source: "../art/interface/WORLDMAP_UI/worldmap-main.png"
    }

    Button {
        id: button1
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

    WorldMapButton {
        id: meadowsButton
        x: 93
        y: 135
        width: 64
        height: 64
        maxRadius: 32
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_Meadows.png"
        }
    }

    Image {
        id: image2
        x: 34
        y: 155
        width: 128
        height: 64
        visible: meadowsButton.visible
        source: "../art/interface/WORLDMAP_UI/WorldMap_Meadows_Script.png"
    }

    WorldMapButton {
        id: moathouseButton
        x: 213
        y: 390
        width: 128
        height: 128
        maxRadius: 24
        smooth: false
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_Moathouse.png"
        }
    }

    Image {
        id: image4
        x: 221
        y: 345
        width: 64
        height: 128
        source: "../art/interface/WORLDMAP_UI/WorldMap_Moathouse_Script.png"
        visible: moathouseButton.visible
    }

    WorldMapButton {
        id: moathouseCaveButton
        x: 286
        y: 438
        width: 64
        height: 64
        maxRadius: 16
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_MoathouseCave.png"
        }
    }

    Image {
        id: image6
        x: 310
        y: 467
        width: 64
        height: 64
        source: "../art/interface/WORLDMAP_UI/WorldMap_MoathouseCave_Script.png"
        visible: moathouseCaveButton.visible
    }

    WorldMapButton {
        id: nulbButton
        x: 353
        y: 97
        width: 128
        height: 128
        maxRadius: 20
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_Nulb.png"
        }
    }

    Image {
        id: image8
        x: 313
        y: 44
        width: 128
        height: 128
        source: "../art/interface/WORLDMAP_UI/WorldMap_Nulb_Script.png"
        visible: nulbButton.visible
    }

    WorldMapButton {
        id: nulbRiverPoolButton
        x: 403
        y: 74
        width: 64
        height: 64
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_NulbRiverPool.png"
        }
        maxRadius: 20
    }

    Image {
        id: image10
        x: 396
        y: 61
        width: 128
        height: 64
        source: "../art/interface/WORLDMAP_UI/WorldMap_NulbRiverPool_Script.png"
        visible: nulbRiverPoolButton.visible
    }

    WorldMapButton {
        id: ogreCaveButton
        x: 187
        y: 111
        width: 128
        height: 128
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_OgreCave.png"
        }
        maxRadius: 30
    }

    Image {
        id: image12
        x: 194
        y: 104
        width: 128
        height: 128
        source: "../art/interface/WORLDMAP_UI/WorldMap_OgreCave_Script.png"
        visible: ogreCaveButton.visible
    }

    WorldMapButton {
        id: templeButton
        x: 394
        y: 195
        width: 84
        height: 84
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_Temple.png"
        }
        maxRadius: 16
    }

    Image {
        id: image14
        x: 317
        y: 225
        width: 128
        height: 64
        source: "../art/interface/WORLDMAP_UI/WorldMap_Temple_Script.png"
        visible: templeButton.visible
    }

    WorldMapButton {
        id: templeWellButton
        x: 415
        y: 165
        width: 64
        height: 64
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_TempleWell.png"
        }
        maxRadius: 14
    }

    Image {
        id: image16
        x: 434
        y: 151
        width: 64
        height: 64
        source: "../art/interface/WORLDMAP_UI/WorldMap_TempleWell_Script.png"
        visible: templeWellButton.visible
    }

    WorldMapButton {
        id: dekloTreesButton
        x: 104
        y: 306
        width: 64
        height: 64
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_DekloTrees.png"
        }
        maxRadius: 20
    }

    Image {
        id: image18
        x: 87
        y: 275
        width: 64
        height: 64
        source: "../art/interface/WORLDMAP_UI/WorldMap_DekloTrees_Script.png"
        visible: dekloTreesButton.visible
    }

    WorldMapButton {
        id: templeHouseButton
        x: 383
        y: 181
        width: 50
        height: 50
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_TempleHouse.png"
        }
        maxRadius: 14
    }

    WorldMapButton {
        id: templeTowerButton
        x: 439
        y: 216
        width: 50
        height: 50
        Image {
            anchors.fill: parent
            source: "../art/interface/WORLDMAP_UI/WorldMap_TempleTower.png"
        }
        maxRadius: 14
    }

    WorldMapButton {
        id: hommletSouth
        x: 117
        y: 458
        width: 32
        height: 32
        maxRadius: 16
    }

    WorldMapButton {
        id: hommletNorth
        x: 105
        y: 423
        width: 32
        height: 32
        maxRadius: 16
    }

    WorldMapButton {
        id: hommletEast
        x: 163
        y: 448
        width: 32
        height: 32
        maxRadius: 16
    }

    Image {
        id: highlightOverlay
        x: 383
        y: 373
        width: 100
        height: 100
        opacity: 0
        source: "../art/interface/WORLDMAP_UI/Worldmap_Ring.png"

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
}
