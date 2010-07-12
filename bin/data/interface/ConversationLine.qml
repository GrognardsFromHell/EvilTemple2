import Qt 4.7

/**
    A line the player can pick in the conversation UI.
  */
Rectangle {
    id: rectangle
    width: 400
    height: 30
    color: "#000000"
    radius: 5

    property alias text : textDisplay.text

    signal clicked

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        anchors.margins: 5
        hoverEnabled: true

        Text {
            id: textDisplay
            anchors.fill: parent

            color: "#ffffff"
            text: "text"
            font.bold: true
            font.pointSize: 12
            font.family: "Fontin"
        }

        onClicked: rectangle.clicked()
    }

    states: [
        State {
            name: "HoverState"
            when: mouseArea.containsMouse

            PropertyChanges {
                target: rectangle
                color: "#8b8b8b"
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            ColorAnimation { properties: "color"; duration: 200 }
        }
    ]
}
