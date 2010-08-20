import Qt 4.7

Rectangle {
    id: root
    width: 431
    height: 233

    color: '#000000'

    property string selectedDeity

    property variant availableDeities : [
        {
            id: 'boccob',
            name: 'asd'
        }

    ]

    onAvailableDeitiesChanged: {
        deityModel.clear();
        availableDeities.forEach(function (deity) {
            deityModel.append({
                'id': deity.id,
                'name': deity.name
            });
        });
    }

    Text {
        x: 0
        y: 5
        width: 433
        height: 17
        color: "#ffffff"
        font.pointSize: 12
        font.family: "Fontin"
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        text: "Choose your Character's Deity"
    }

    ListModel {
        id: deityModel
    }

    Component {
        id: deityDelegate
        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 23

            Item {
                anchors.horizontalCenter: parent.horizontalCenter

                width: 174
                height: 23
                clip: true
                Image {
                    source: '../art/interface/pc_creation/deitybox.png'
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 5
                fontSize: 11
                text: model.name
                normalImage: (selectedDeity == model.id) ? 'art/interface/pc_creation/deity_button_selected.png'
                                                         : 'art/interface/pc_creation/deity_button.png'
                hoverImage: 'art/interface/pc_creation/deity_button_hovered.png'
                pressedImage: 'art/interface/pc_creation/deity_button_clicked.png'
                disabledImage: 'art/interface/pc_creation/deity_button_disabled.png'
                onClicked: selectedDeity = model.id
            }
        }
    }

    ListView {
        x: 0
        y: 32
        width: 431
        height: 201
        clip: true
        model: deityModel
        boundsBehavior: "StopAtBounds"
        delegate: deityDelegate
    }

}
