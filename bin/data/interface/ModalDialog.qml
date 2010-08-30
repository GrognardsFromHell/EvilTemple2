import Qt 4.7

Rectangle {

    id: root

    property string title : 'Message'

    property string message : 'Please read this message'

    signal okay

    signal cancel

    width: gameView.viewportSize.width
    height: gameView.viewportSize.height

    color: '#7f000000'

    Image {
        source: '../art/interface/generic/genericdialogue_nocheckbox.png'
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        MouseArea {
            anchors.fill: parent
            drag.target: parent
            drag.minimumX: 0
            drag.maximumX: root.width - parent.width
            drag.minimumY: 0
            drag.maximumY: root.height - parent.height
            drag.axis: Drag.XandYAxis
        }

        StandardText {
            x: 0
            y: 17
            anchors.leftMargin: 1
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: "AlignHCenter"
            text: title
            font.bold: true
            color: '#333333'
        }

        StandardText {
            x: 0
            y: 16
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: "AlignHCenter"
            text: title
            anchors.rightMargin: 0
            anchors.leftMargin: 0
            font.bold: true
        }

        StandardText {
            x: 24
            y: 45
            width: 244
            height: 116
            text: message
            wrapMode: "WrapAtWordBoundaryOrAnywhere"
        }

        Button {
            x: 28
            y: 170
            text: 'Okay'
            onClicked: root.okay()
        }

        CancelButton {
            x: 152
            y: 170
            onClicked: root.cancel()
        }

    }

}
