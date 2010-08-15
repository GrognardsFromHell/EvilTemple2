import Qt 4.7

Rectangle {
    id: root
    width: 431
    height: 233

    color: '#000000'

    Text {
        x: 5
        y: 5
        width: 427
        height: 17
        color: "#ffffff"
        font.pointSize: 12
        font.family: "Fontin"
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        text: "Choose your Character's Race"
    }

    property string selectedRace : ''

    property bool complete : false

    Flickable {
        anchors.fill: parent
        anchors.margins: 5
        anchors.topMargin: 25
        contentHeight: raceColumn.height
        clip: true
        boundsBehavior: "StopAtBounds"
        Column {
            id: raceColumn
            anchors.horizontalCenter: parent.horizontalCenter
            Repeater {
                model: ['human', 'dwarf', 'elf', 'gnome', 'half-elf', 'half-orc', 'halfling', 'deep-dwarf',
                    'aquatic-elf', 'svirfneblin', 'tallfellow', 'derro', 'drow', 'forest-gnome',
                'deep-halfling', 'duergar', 'gray-elf', 'mountain-dwarf', 'wild-elf', 'wood-elf']
                Image {
                    source: '../art/interface/pc_creation/rollbox.png'
                    CreateCharacterButtonRight {
                        x: 5
                        y: 5
                        active: selectedRace == modelData
                        text: modelData
                        onClicked: {
                            selectedRace = modelData;
                            complete = true;
                        }
                    }
                }
            }
        }
    }
}
