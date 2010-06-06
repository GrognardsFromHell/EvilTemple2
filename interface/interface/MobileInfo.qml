import Qt 4.7

Item {
    width: 400
    height: 480

    Rectangle {
        id: background
        opacity: 0.5
        anchors.fill: parent
        color: "#b7b7b7"
        radius: 4
        clip: false
        border.width: 4
    }

    property string title : 'Title';

    signal closeClicked;

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
        id: listview
        x: 4
        y: 41
        width: 392
        height: 435
        model: listModel
        delegate: itemDelegate
        clip: true
    }

    Text {
        id: header
        x: 4
        y: 4
        width: 358
        height: 30
        text: title
        font.bold: true
        font.pointSize: 20
        font.family: "Handserif"
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

    MouseArea {
        x: 364
        y: 4
        width: 32
        height: 30

        Image {
            anchors.fill: parent
            source: "../art/interface/cursors/Sword_INVALID.png"
        }

        onClicked: closeClicked()
    }

}
