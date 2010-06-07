import Qt 4.6

MouseArea {
    id: mousearea2
    width: 640
    height: 480

    property string title : 'Title';
    signal closeClicked;

    Rectangle {
        id: background
        opacity: 0.5
        anchors.fill: parent
        color: "#b7b7b7"
        radius: 4
        clip: false
        border.width: 4
    }

    Text {
        id: header
        x: 4
        y: 4
        width: 358
        height: 30
        text: title
        font.bold: true
        font.pointSize: 20
        font.family: "Handserif"
    }

    MouseArea {
        id: mousearea1
        anchors.right: parent.right
        width: 32
        height: 30
        anchors.rightMargin: 6
        anchors.top: parent.top
        anchors.topMargin: 6

        Image {
            anchors.bottomMargin: 0
            anchors.leftMargin: 0
            anchors.fill: parent
            anchors.rightMargin: 0
            anchors.topMargin: 0
            source: "../art/interface/cursors/Sword_INVALID.png"
        }

        onClicked: closeClicked()
    }

}
