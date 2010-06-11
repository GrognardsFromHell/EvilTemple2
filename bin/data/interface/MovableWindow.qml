import Qt 4.7

MouseArea {
    id: window
    width: 640
    height: 480
    x: (gameView.viewportSize.width - width) / 2
    y: (gameView.viewportSize.height - height) / 2

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
        color: "#333333"
        radius: 4
        clip: true
        border.width: 4
        Image {
            anchors.margins: 4
            anchors.fill: parent
            source: 'CharScreen.jpg'
            fillMode: 'Stretch'
        }
    }

    Text {
        id: header
        x: 4
        y: 4
        width: 358
        height: 30
        text: title
        anchors.right: closeButton.left
        anchors.rightMargin: 6
        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.top: parent.top
        anchors.topMargin: 6
        font.bold: true
        font.pointSize: 20
        font.family: "Handserif"
        color: '#FFFFFF'
    }

    Item {
        id: viewport
        anchors.margins: 6
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    Button {
        id: closeButton
        anchors.margins: 6
        anchors.right: parent.right
        anchors.top: parent.top
        normalImage: 'interface/Close_Up.png'
        hoverImage: 'interface/Close_Hover.png'
        pressedImage: 'interface/Close_Pressed.png'
        text: ''
        onClicked: {
            closeClicked();
        }
    }

}
