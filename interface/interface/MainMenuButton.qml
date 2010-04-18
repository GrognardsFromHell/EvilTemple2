import Qt 4.6

MouseArea {

    property alias text: buttonLabel.text

    width: buttonLabel.width
    height: buttonLabel.height

    hoverEnabled: true

    onEntered: if (!focus) state = "hover";
    onExited: if (!focus) state = '';
    onFocusChanged: if (focus) {
        state = 'focus'
    } else if (containsMouse) {
        state = 'hover';
    } else {
        state = '';
    }

    Text {
        id: buttonLabel
        text: "Button"
        font.pointSize: 42
        font.family: "Handserif"
        color: "white"
    }

    states: [
        State {
            name: "hover"

            PropertyChanges {
                target: buttonLabel
                color: "#2121db"
            }
        },
        State {
            name: "pressed"

            PropertyChanges {
                target: buttonLabel
                color: "#f50a0a"
            }
        },
        State {
            name: "focus"

            PropertyChanges {
                target: buttonLabel
                color: "#ffcd03"
            }
        }
    ]
}
