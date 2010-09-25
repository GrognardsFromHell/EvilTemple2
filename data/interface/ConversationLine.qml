import Qt 4.7

/**
    A line the player can pick in the conversation UI.
  */
MouseArea {
    id: mouseArea
    width: 400
    height: textDisplay.height + 10 // Include a margin of 10
    hoverEnabled: true

    property alias text : textDisplay.text

    Rectangle {
        id: rectangle
        anchors.fill: parent

        radius: 5
        opacity: 0.5
        gradient: Gradient {
            GradientStop {
                position: 0
                id: stop1
                color: "#555555"
            }

            GradientStop {
                position: 1
                id: stop2
                color: "#292929"
            }
        }
    }

    Text {
        id: textDisplay

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5
        y: 5

        color: "#ffffff"
        text: "text"
        wrapMode: "WordWrap"
        font.bold: true
        font.pointSize: 12
        font.family: "Fontin"
    }

    states: [
        State {
            name: "HoverState"
            when: mouseArea.containsMouse

            PropertyChanges { target: rectangle; opacity: 1 }
            PropertyChanges { target: stop1; color: '#00b7e8' }
            PropertyChanges { target: stop2; color: '#005993' }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            NumberAnimation { properties: "opacity"; duration: 200 }
        }
    ]
}
