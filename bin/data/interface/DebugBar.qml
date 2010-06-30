import Qt 4.7

Item
{
    signal debugEvent(string name);

    height: childrenRect.height
    width: childrenRect.width

    x: (gameView.viewportSize.width - width) / 2
    y: gameView.viewportSize.height - height

    Row {
        id: buttonRow
        x: 5
        y: 5
        height: 32
        spacing: 5

        MouseArea {
            width: childrenRect.width
            height: childrenRect.height
            hoverEnabled: true
            Text {
                color: parent.containsMouse ? '#0000FF' : '#FFFFFF'
                font.pointSize: 12
                text: 'ParticleSystem'
            }
            onClicked: debugEvent('spawnParticleSystem')
        }
        MouseArea {
            width: childrenRect.width
            height: childrenRect.height
            hoverEnabled: true
            Text {
                color: parent.containsMouse ? '#0000FF' : '#FFFFFF'
                font.pointSize: 12
                text: 'Load Map'
            }
            onClicked: debugEvent('loadMap')
        }
    }

    Rectangle {
        x: 0
        y: 0
        width: buttonRow.width + 10
        height: buttonRow.height + 10
        opacity: 0.5
        color: '#333333'
    }
}
