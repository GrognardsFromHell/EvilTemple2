import Qt 4.7

MovableWindow {
    id: root
    title: ''
    width: 640
    height: 480

    signal buyItem(string guid)

    property alias money : moneyDisplay.money

    property variant merchantItems : [{
            iconPath: 'art/interface/inventory/Sword_2-Handed3_Icon.png',
            description: 'Some Sword',
            worth: 1000,
            weight: 10
        }]

    onMerchantItemsChanged: {
        merchantInventoryModel.clear();
        merchantItems.forEach(function (item) {
            merchantInventoryModel.append({
                iconPath: item.iconPath,
                description: item.description,
                worth: item.worth,
                quantity: item.quantity,
                weight: item.weight,
                guid: item.guid
            });
        });
    }

    ListModel {
        id: merchantInventoryModel
    }

    ListModel {
        id: playerInventoryModel
    }

    Rectangle {
        id: merchantInventoryBackground
        x: 10
        width: root.width / 2 - 10 - 5
        anchors.top: merchantNameText.bottom
        anchors.topMargin: 10
        anchors.bottom: moneyDisplay.top
        anchors.bottomMargin: 10

        opacity: 0.5
        radius: 5
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#333333"
            }

            GradientStop {
                position: 1
                color: "#000000"
            }
        }
    }

    Rectangle {
        id: playerInventoryBackground
        anchors.top: playerNameText.bottom
        anchors.topMargin: 10
        anchors.bottom: moneyDisplay.top
        anchors.bottomMargin: 10
        x: parent.width / 2 + 5
        width: parent.width - x - 10

        radius: 5
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#333333"
            }

            GradientStop {
                position: 1
                color: "#000000"
            }
        }
        opacity: 0.5
    }

    Component {
        id: delegate

        InventoryItem {
            iconPath: model.iconPath
            quantity: model.quantity ? model.quantity : 1
            location: model.location ? model.location : 0
            description: model.description
            magical: model.magical ? true : false
            weight: model.weight
            worth: model.worth

            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined

            Image {
                source: 'buyarrow_off.png'
                x: parent.width - 48
                y: (parent.height - height) / 2
                width: 32
                height: 32
                smooth: true
                visible: parent.containsMouse
            }

            onDoubleClicked: buyItem(model.guid)
        }
    }

    ListView {
        id: merchantInventoryView
        anchors.fill: merchantInventoryBackground
        anchors.margins: 10
        model: merchantInventoryModel
        delegate: delegate
        clip: true
        spacing: 5
    }

    StandardText {
        id: merchantNameText
        x: 12
        y: 15
        text: 'Merchant Name'
        font.bold: true
    }

    StandardText {
        id: playerNameText
        y: 15
        text: 'Player Name'
        font.bold: true
        anchors.left: playerInventoryBackground.left
    }


    MoneyDisplay {
        id: moneyDisplay
        anchors.left: playerInventoryBackground.left
        anchors.right: playerInventoryBackground.right
        anchors.bottom: root.bottom
        anchors.bottomMargin: 10
    }

}
