import Qt 4.7
import "WorldMapTravel.js" as WorldMapTravel
import "WorldMap.js" as WorldMap
import "Utilities.js" as Utilities

MovableWindow {
    id: root
    width: 792
    height: 586
    title: "Worldmap"

    signal travelRequested(string area)
    signal travelFinished

    property variant areas : []

    onAreasChanged: WorldMap.setAreas(areas)

    function showYouAreHere(x, y) {
        youAreHere.visible = true;
        youAreHere.x = x - youAreHere.width / 2;
        youAreHere.y = y - youAreHere.height / 2;
    }

    function hideYouAreHere() {
        youAreHere.visible = false;
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
        interval: 15
        repeat: true
        onTriggered: WorldMapTravel.doTravelStep()
    }

    function refreshHighlight(mouseX, mouseY) {
        var centerX, centerY, radius;
        var worldmapButton;

        for (var i = 0; i < children.length; ++i) {
            var child = root.children[i];

            radius  = child.radius;

            if (!radius || !child.visible)
                continue;

            centerX = (child.x + child.width / 2);
            centerY = (child.y + child.height / 2);

            var diffX = mouseX - centerX;
            var diffY = mouseY - centerY;
            var distance = Math.sqrt(diffX * diffX + diffY * diffY);

            if (distance <= radius) {
                highlightOverlay.x = centerX - radius;
                highlightOverlay.y = centerY - radius;
                highlightOverlay.width = radius * 2;
                highlightOverlay.height = radius * 2;
                worldmapButton = child;
                break;
            }
        }

        if (worldmapButton) {
            highlightOverlay.state = 'visible';
            tooltip.text = worldmapButton.name;
            tooltip.x = centerX - tooltip.width / 2;
            tooltip.y = centerY + radius;
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
