import Qt 4.7

Rectangle {
    id: root
    width: 431
    height: 233

    color: '#000000'

    property bool complete : false
    property string gender

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
        text: "Choose your Character's Gender"
    }

    Image {
        x: 112
        y: 76
        source: '../art/interface/pc_creation/male.png'
    }

    Image {
        x: 261
        y: 76
        source: '../art/interface/pc_creation/female.png'
    }

    Image {
        x: 79
        y: 144
        source: '../art/interface/pc_creation/buttonbox.png'
        CreateCharacterButtonRight {
            x: 5
            y: 5
            text: 'MALE'
            active: gender == 'male'
            onClicked: {
                gender = 'male';
                complete = true;
            }
        }
    }

    Image {
        x: 228
        y: 144
        source: '../art/interface/pc_creation/buttonbox.png'
        CreateCharacterButtonRight {
            x: 5
            y: 5
            text: 'FEMALE'
            active: gender == 'female'
            onClicked: {
                gender = 'female';
                complete = true;
            }
        }
    }
}
