import Qt 4.7

MovableWindow {
    id: movablewindow1
    width: 640
    height: 480

    signal commandIssued(string command)

    function appendResult(text) {
        logText.text += text + "<br><br>\n\n";
        scrollDown();
    }

    function issueCommand() {
        var command = commandLine.text;
        commandLine.text = '';

        logText.text += '> ' + command + '<br>\n';
        commandIssued(command);
        scrollDown();
    }

    function scrollDown() {
        flickable1.contentY = flickable1.contentHeight - flickable1.height
    }

    data: [
        Flickable {
            id: flickable1
            anchors.fill: rectangle1
            anchors.margins: 5
            contentHeight: logText.height
            contentWidth: width
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            Text {
                id: logText
                color: "#ffffff"
                text: ""
                font.pointSize: 11
                font.family: "Consolas"
                width: parent.width
                wrapMode: "WrapAtWordBoundaryOrAnywhere"
                textFormat: "RichText"
            }
        },
        Rectangle {
            id: rectangle1
            x: 8
            y: 40
            width: 624
            height: 397
            color: "#000000"
            radius: 5
            border.color: "#ffffff"
            opacity: 0.4
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 10
        },
        Rectangle {
            id: rectangle2
            x: 118
            y: 443
            width: 515
            height: 26
            color: "#000000"
            radius: 5
            border.color: "#ffffff"
            opacity: 0.4
            anchors.left: text1.right
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
        },
        TextInput {
            id: commandLine
            color: "#ffffff"
            text: ""
            cursorVisible: false
            font.pointSize: 11
            font.family: "Consolas"
            anchors.left: rectangle2.left
            anchors.leftMargin: 4
            anchors.right: rectangle2.right
            anchors.rightMargin: 4
            anchors.verticalCenter: rectangle2.verticalCenter
            Keys.onEnterPressed: issueCommand()
            Keys.onReturnPressed: issueCommand()
        },
        Text {
            id: text1
            x: 7
            y: 446
            color: "#ffffff"
            text: "Command:"
            anchors.verticalCenter: rectangle2.verticalCenter
            font.pointSize: 12
            font.family: "Fontin"
            anchors.left: parent.left
            anchors.leftMargin: 10
        }
    ]
    title: "Console"
}
