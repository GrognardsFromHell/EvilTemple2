import Qt 4.6

Rectangle {
    id: consoleRectangle
    width: 640
    height: 0
    z: 100
    clip: true
    color: "transparent"

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top

    // Adds a message to the console
    // Type can be one of the following:
    // - error
    // - warning
    // anything else is treated as a normal message.
    function addMessage(message, type) {
        if (type == 'error') {
            message = '<font color="red">' + message + '</font>';
        }

        consoleLog.text += message + "<br>";
    }

    // Toggles the console, is called by the C++ program to show/hide the console
    // if the corresponding key event was ignored by Qml since no item had focus.
    function toggle() {
        state = (state == '') ? 'shown' : '';
    }

    Rectangle {
        color: "white"
        opacity: 0.25
        anchors.fill: parent
        z: 0
    }

    TextInput {
        id: inputLine
        x: 6
        y: 454
        width: 628
        height: 20
        z: 1
        color: "white"
        text: ""
        cursorVisible: false
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.left: parent.left
        anchors.leftMargin: 6
    }

    Text {
        id: consoleLog
        x: 6
        y: 6
        z: 1
        width: 628
        textFormat: 'StyledText'
        height: 446
        wrap: true
        text: ""
        color: "white"
        anchors.bottom: inputLine.top
        anchors.bottomMargin: 6
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.top: parent.top
        anchors.topMargin: 6        
    }

    states: [
        State {
            name: "shown"

            PropertyChanges {
                target: consoleRectangle
                height: 400
            }
        }
    ]

    transitions: [
    Transition {
        NumberAnimation {
            properties: "height"
            easing.type: "OutBounce"
            duration: 250
        }
    }
    ]

}
