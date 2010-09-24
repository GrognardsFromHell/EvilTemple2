import Qt 4.7

Image {

    // URL of the portrait image
    property string portrait : 'art/interface/portraits/TempMan.png'

    // Whether this character is part of the selection
    property bool selected : false

    property real health : 0.5

    signal clicked
    signal doubleClicked
    signal rightClicked

    id: frame
    width: 112
    height: 108
    fillMode: "Stretch"
    source: "../art/interface/PARTY_UI/Character Portrait Frame.png"

    Image {
        id: portraitImage
        x: 16
        y: 11
        width: 80
        height: 70
        smooth: true
        fillMode: "PreserveAspectFit"
        source: '../' + portrait

        Rectangle {
            anchors.fill: parent
            color: '#00000000'
            border.color: {
                if (selected) {
                    return '#0000FF';
                } else {
                    return mouseArea.containsMouse ? '#CCCCFF' : '#00000000';
                }
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onClicked: if (mouse.button == Qt.LeftButton)
                            frame.clicked()
                        else
                            frame.rightClicked()

            onDoubleClicked: frame.doubleClicked()
        }

        Rectangle {
            x: 0
            y: 74
            color: 'red'
            width: health * 80
            height: 6
        }
    }

}
