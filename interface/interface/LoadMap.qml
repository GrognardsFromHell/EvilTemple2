import Qt 4.7

MovableWindow {
    width: 500
    height: 300
    title: 'Load Map'

    signal mapSelected(string mapName)

    ListModel {
        id: mapModel
    }

    ListView {
        anchors.fill: parent
        anchors.margins: 6
        anchors.topMargin: 45
        clip: true
        model: mapModel
        delegate: MouseArea {
            id: mouseArea
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height + 2
            hoverEnabled: true
            Row {
                Text {
                    text: name
                    width: 200
                    font.family: "Fontin"
                    font.pointSize: 12
                    color: mouseArea.containsMouse ? '#ff0000':'#000000'
                }
                Text {
                    text: dir
                    font.family: "Fontin"
                    font.pointSize: 12
                    color: mouseArea.containsMouse ? '#ff0000':'#000000'
                }
            }
            onClicked: mapSelected(dir)
        }
    }

    function setMapList(maps) {
        mapModel.clear();

        for (var i = 0; i < maps.length; ++i) {
            mapModel.append(maps[i]);
        }
    }


}
