import Qt 4.7

MovableWindow {
    width: 400
    height: 480

    property bool hasInventory : false;

    signal openInventory;
    signal openAnimations;

    ListModel {
        id: listModel
        ListElement {
            name: 'Test'
            value: 'test'
        }
    }

    Component {
        id: itemDelegate
        Text {
           anchors.left: parent.left
           anchors.right: parent.right
           text: '<b>' + name + ':</b> ' + value
           font.pointSize: 12
           font.family: "Fontin"
           wrapMode: Text.WrapAtWordBoundaryOrAnywhere
           color: '#FFFFFF'
        }
    }

    ListView {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: buttonRow.top
        anchors.margins: 6
        anchors.topMargin: 45
        model: listModel
        delegate: itemDelegate
        clip: true
    }

    Image {
        id: portrait
        width: 64
        height: 64
        anchors.right: parent.left
        anchors.top: parent.top
        visible: false
        smooth: true
    }

    Row {
        id: buttonRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 6
        spacing: 6

        Image {
            source: 'ButtonBackdrop.png'
            width: inventoryButton.width + 8
            height: inventoryButton.height + 8
            Button {
                id: inventoryButton
                enabled: hasInventory
                x: 4
                y: 4
                text: 'Inventory'
                onClicked: openInventory()
            }
        }

        Image {
            source: 'ButtonBackdrop.png'
            width: animationsButton.width + 8
            height: animationsButton.height + 8
            Button {
                id: animationsButton
                x: 4
                y: 4
                text: 'Animations'
                onClicked: openAnimations()
            }
        }
    }

    function setPortrait(filename) {
        if (filename == null) {
            portrait.visible = false;
            portrait.visible = false;
        } else {
            portrait.source = '../' + filename;
            portrait.visible = true;
        }
    }

    function setItems(items) {
        listModel.clear();

        for (var i = 0; i < items.length; ++i) {
            var item = items[i];
            listModel.append({
                name: item[0],
                value: item[1]
            });
        }
    }

}
