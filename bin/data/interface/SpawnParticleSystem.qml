import Qt 4.7

MovableWindow {
    width: 500
    height: 150
    title: 'Spawn Particle System'

    signal spawnParticleSystem(string name)

    Column {
        anchors.fill: parent
        anchors.margins: 6
        anchors.topMargin: 45

        Rectangle {
            width: 400
            height: 25
            color: '#333333'

            TextInput {
                id: partSysName
                anchors.fill: parent
                anchors.margins: 2
                font.family: "Fontin"
                font.pointSize: 12
                color: '#ffffff'
            }
        }

        Image {
            source: 'ButtonBackdrop.png'
            width: spawnButton.width + 8
            height: spawnButton.height + 8
            Button {
                id: spawnButton
                x: 4
                y: 4
                text: 'Spawn'
                onClicked: spawnParticleSystem(partSysName.text)
            }
        }
    }

}
