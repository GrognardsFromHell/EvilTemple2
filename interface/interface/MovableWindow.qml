import Qt 4.7

MouseArea {
    id: window
    width: 640
    height: 480

    drag.target: window
    drag.axis: Drag.XandYAxis
    drag.minimumX: 0
    drag.maximumX: gameView.viewportSize.width - width
    drag.minimumY: 0
    drag.maximumY: gameView.viewportSize.height - height

    property string title : 'Title';

    signal closeClicked;

    Rectangle {
        id: background
        opacity: 0.5
        anchors.fill: parent
        color: "#FFFFFF"
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
        anchors.right: mousearea1.left
        anchors.rightMargin: 6
        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.top: parent.top
        anchors.topMargin: 6
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

    Item {
        id: viewport
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.margins: 6
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

}
