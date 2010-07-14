import Qt 4.7

Rectangle {
    id: root
    width: 618
    height: 36
    color: "#2b2b2b"

    property real progress : 0.5

    Rectangle {
        id: progressBar
        anchors.margins: 3
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: progress * (parent.width - 10)
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#ffffff"
            }

            GradientStop {
                position: 1
                color: "#cccccc"
            }
        }

    }
}
