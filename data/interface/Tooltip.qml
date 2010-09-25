import Qt 4.7

Item {
    id: tooltip
    width: tooltipText.width + 10
    height: tooltipText.height + 10
    z: 5

    property alias text : tooltipText.text

    property bool shown : false

    opacity: 0

    Rectangle {
        opacity: 0.5
        anchors.fill: parent
        radius: 5
        clip: true
        z: 5
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#333333"
            }

            GradientStop {
                position: 1
                color: "#000000"
            }
        }
    }

    Text {
        x: 5
        y: 5
        z: 6
        id: tooltipText
        text: 'Tooltip Text'
        font.family: 'Fontin'
        font.pointSize: 11
        color: '#FFFFFF'
    }

    states: State {
        name: "shown";
        when: tooltip.shown && (tooltip.text !== "");
        PropertyChanges { target: tooltip; opacity: 1 }
    }

    transitions: [
        Transition {
            NumberAnimation {
                duration: 175;
                target: tooltip;
                property: "opacity";
            }
        }
    ]

}
