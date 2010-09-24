import Qt 4.7

MovableWindow {
    id: root
    title: ''
    width: 640
    height: 480

    signal buyItem(string guid)
    signal sellItem(string guid)

    property alias money : moneyDisplay.money
    property alias merchantName : merchantNameText.text
    property alias playerName : playerNameText.text
    property string playerPortrait : 'art/interface/portraits/TempMan.png'
    property string merchantPortrait : 'art/interface/portraits/TempMan.png'

    property variant merchantItems : [{
            iconPath: 'art/interface/inventory/Sword_2-Handed3_Icon.png',
            description: 'Some Sword',
            worth: 1000,
            weight: 10
        }]

    property variant playerItems : [{
            iconPath: 'art/interface/inventory/Sword_2-Handed3_Icon.png',
            description: 'Some Sword',
            worth: 1000,
            weight: 10
        }]

    function copyItem(item) {
        return {
            iconPath: item.iconPath,
            description: item.description,
            worth: item.worth,
            quantity: item.quantity,
            weight: item.weight,
            guid: item.guid
        };
    }

    onMerchantItemsChanged: {
        merchantInventoryModel.clear();
        merchantItems.forEach(function (item) {
            merchantInventoryModel.append(copyItem(item));
        });
    }

    onPlayerItemsChanged: {
        playerInventoryModel.clear();
        playerItems.forEach(function (item) {
            playerInventoryModel.append(copyItem(item));
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


    Image {
        id: buyArrowImage
        visible: false
        source: 'buyarrow_over.png'
        x: (parent.width - width) / 2 - 10
        width: 64
        height: 64
        smooth: true
        opacity: 0.75
        z: 1000
    }

    Image {
        id: sellArrowImage
        visible: false
        source: 'sellarrow_over.png'
        x: (parent.width - width) / 2 - 10
        width: 64
        height: 64
        smooth: true
        opacity: 0.75
        z: 1000
    }

    Component {
        id: buyDelegate

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

            onEntered: {
                buyArrowImage.y = merchantInventoryView.y + y - merchantInventoryView.contentY + (height - 64) / 2;
                buyArrowImage.visible = true;
            }

            onMousePositionChanged: {
                buyArrowImage.y = merchantInventoryView.y + y - merchantInventoryView.contentY + (height - 64) / 2;
            }

            onExited: {
                buyArrowImage.visible = false;
            }

            onDoubleClicked: buyItem(model.guid)
        }
    }


    Component {
        id: sellDelegate

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


            onEntered: {
                sellArrowImage.y = playerInventoryView.y + y - playerInventoryView.contentY + (height - 64) / 2;
                sellArrowImage.visible = true;
            }

            onMousePositionChanged: {
                sellArrowImage.y = playerInventoryView.y + y - playerInventoryView.contentY + (height - 64) / 2;
            }

            onExited: {
                sellArrowImage.visible = false;
            }

            onDoubleClicked: sellItem(model.guid)
        }
    }

    ListView {
        id: merchantInventoryView
        anchors.fill: merchantInventoryBackground
        anchors.margins: 10
        model: merchantInventoryModel
        delegate: buyDelegate
        clip: true
        spacing: 5
    }

    ListView {
        id: playerInventoryView
        anchors.fill: playerInventoryBackground
        anchors.margins: 10
        model: playerInventoryModel
        delegate: sellDelegate
        clip: true
        spacing: 5
    }

    Rectangle {
        id: merchantPortraitImage
        width: 32
        height: 28
        anchors.verticalCenter: merchantNameText.verticalCenter
        anchors.left: merchantInventoryBackground.left
        color: 'black'
        anchors.leftMargin: 5

        Image {
            anchors.fill: parent
            anchors.margins: 1
            source: '../' + merchantPortrait
            smooth: true
        }
    }

    StandardText {
        id: merchantNameText
        anchors.left: merchantPortraitImage.right
        anchors.leftMargin: 10
        y: 15
        text: 'Merchant Name'
        font.bold: true
    }

    Rectangle {
        id: playerPortraitImage
        width: 32
        height: 28
        anchors.verticalCenter: playerNameText.verticalCenter
        anchors.left: playerInventoryBackground.left
        anchors.leftMargin: 5
        color: 'black'

        Image {
            anchors.fill: parent
            anchors.margins: 1
            source: '../' + playerPortrait
            smooth: true
        }
    }

    StandardText {
        id: playerNameText
        y: 15
        text: 'Player Name'
        anchors.leftMargin: 10
        font.bold: true
        anchors.left: playerPortraitImage.right
    }


    MoneyDisplay {
        id: moneyDisplay
        anchors.left: playerInventoryBackground.left
        anchors.right: playerInventoryBackground.right
        anchors.bottom: root.bottom
        anchors.bottomMargin: 10
    }

}
