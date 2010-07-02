import Qt 4.7

Item
{
    signal debugEvent(string name);

    height: childrenRect.height
    width: childrenRect.width

    x: (gameView.viewportSize.width - width) / 2
    y: gameView.viewportSize.height - height

    Rectangle {
        x: 0
        y: 0
        width: buttonRow.width + 10
        height: buttonRow.height + 10
        opacity: 0.5
        color: '#333333'
    }

    Row {
        id: buttonRow
        x: 5
        y: 5
        height: 32
        spacing: 10

        MouseArea {
            width: childrenRect.width
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            hoverEnabled: true
            Text {
                color: parent.containsMouse ? '#00a9f8' : '#FFFFFF'
                text: "Spawn PartSys"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: 12
                font.bold: true
                font.family: "Fontin"
            }
            onClicked: debugEvent('spawnParticleSystem')
        }
        MouseArea {
            width: childrenRect.width
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            hoverEnabled: true
            Text {
                color: parent.containsMouse ? '#00a9f8' : '#FFFFFF'
                text: 'Load Map'
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: 12
                font.bold: true
                font.family: "Fontin"
            }
            onClicked: debugEvent('loadMap')
        }
    }

}
