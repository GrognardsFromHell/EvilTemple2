import Qt 4.7

Rectangle {
    id: root
    width: 431
    height: 233

    color: '#000000'

    property variant classes : [
        { 'id': 'bard', 'name': 'Bard' }
    ]

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
        text: "Choose your Character's Class"
    }

    property string selectedClass : ''

    Flickable {
        anchors.fill: parent
        anchors.margins: 5
        anchors.topMargin: 25
        contentHeight: classColumn.height
        clip: true
        boundsBehavior: "StopAtBounds"
        Column {
            id: classColumn
            anchors.horizontalCenter: parent.horizontalCenter
            Repeater {
                model: classes
                Image {
                    source: '../art/interface/pc_creation/rollbox.png'
                    CreateCharacterButtonRight {
                        x: 5
                        y: 5
                        active: selectedClass == modelData.id
                        text: modelData.name
                        onClicked: {
                            selectedClass = modelData.id;
                        }
                    }
                }
            }
        }
    }
}
