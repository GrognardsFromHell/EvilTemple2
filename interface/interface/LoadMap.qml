import Qt 4.7

Item {
    id: rectangle1
    width: 500
    height: 300

    signal mapSelected(string mapName);
    signal closeClicked;

    Rectangle {
        id: background
        opacity: 0.5
        anchors.fill: parent
        color: "#b7b7b7"
        radius: 4
        clip: false
        border.width: 4
    }

    ListModel {
        id: mapModel
    }

    ListView {
        id: listview
        x: 6
        y: 44
        clip: true
        width: 628
        height: 430
        model: mapModel
        anchors.top: text1.bottom
        anchors.topMargin: 9
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.right: parent.right
        anchors.rightMargin: 6
        delegate: MouseArea {
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height + 2
            Row {
                Text {
                    text: name
                    width: 200
                    font.family: "Fontin"
                    font.pointSize: 12
                }
                Text {
                    text: dir
                    font.family: "Fontin"
                    font.pointSize: 12
                }
            }
            onClicked: mapSelected(dir)
        }
    }

    MouseArea {
        x: 464
        y: 4
        width: 32
        height: 30

        Image {
            anchors.fill: parent
            source: "../art/interface/cursors/Sword_INVALID.png"
        }

        onClicked: closeClicked()
    }

    Text {
        id: text1
        x: 6
        y: 6
        width: 123
        height: 29
        text: "Load Map"
        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.top: parent.top
        anchors.topMargin: 6
        font.bold: true
        font.pointSize: 22
        font.family: "Handserif"
    }

    function setMapList(maps) {
        mapModel.clear();

        for (var i = 0; i < maps.length; ++i) {
            var map = maps[i];
            mapModel.append({
                name: map[0],
                dir: map[1]
            });
        }

    }




}
