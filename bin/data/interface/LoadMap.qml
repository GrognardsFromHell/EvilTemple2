import Qt 4.7

MovableWindow {
    width: 700
    height: 300
    title: 'Load Map'

    property variant mapList : {
        maps: [{mapId: 1000, name: 'Some Map', dir: 'Some Dir'}]
    }

    signal mapSelected(variant map)

    ListModel {
        id: listModel
    }

    onMapListChanged: {
        listModel.clear();
        for (var i = 0; i < mapList.length; ++i) {
            listModel.append(mapList[i]);
        }
    }

    Component {
        id: delegate

        MouseArea {
            id: mouseArea
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height + 2
            hoverEnabled: true
            Row {
                Text {
                    text: model.name
                    width: 350
                    font.family: "Fontin"
                    font.pointSize: 12
                    color: mouseArea.containsMouse ? '#00a9f8':'#ffffff'
                }
                Text {
                    text: model.dir
                    font.family: "Fontin"
                    font.pointSize: 12
                    color: mouseArea.containsMouse ? '#00a9f8':'#ffffff'
                }
            }
            onClicked: mapSelected(dir)
        }
    }

    ListView {
        anchors.fill: parent
        anchors.margins: 6
        anchors.topMargin: 45
        clip: true
        model: listModel
        delegate: delegate
    }

}
