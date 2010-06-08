import Qt 4.7

MovableWindow {
    width: 500
    height: 480
    title: 'Inventory'

    Component {
        id: itemDelegate
        Row {
            spacing: 5
            Image {
                source: '../' + iconPath
            }
            Column {
                spacing: 2
                Text {
                    text: description
                    font.family: 'Fontin'
                    font.pointSize: 12
                    font.weight: Font.Bold
                }
                Row {
                    spacing: 5
                    Text {
                        text: "<i>Quantity:</i> " + quantity
                        font.family: 'Fontin'
                        font.pointSize: 12
                    }
                    Text {
                        text: "<i>Location:</i> " + location
                        font.family: 'Fontin'
                        font.pointSize: 12
                    }
                }
            }
        }
    }

    ListModel {
        id: listModel
    }

    ListView {
        anchors.fill: parent
        anchors.margins: 6
        anchors.topMargin: 45
        anchors.bottomMargin: 40
        model: listModel
        delegate: itemDelegate
        clip: true
    }

    MoneyDisplay {
        id: moneyDisplay
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 6
    }

    function setInventory(inventory) {
        var items = inventory.items;

        moneyDisplay.platinum = inventory.platinum;
        moneyDisplay.gold = inventory.gold;
        moneyDisplay.silver = inventory.silver;
        moneyDisplay.copper = inventory.copper;

        listModel.clear();
        for (var i = 0; i < items.length; ++i) {
            listModel.append(items[i]);
        }

    }

}
