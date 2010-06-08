import Qt 4.7

MovableWindow {
    width: 400
    height: 480
    x: (gameView.viewportSize.width - width) / 2
    y: (gameView.viewportSize.height - height) / 2

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
        }
    }

    ListView {
        anchors.fill: parent
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
