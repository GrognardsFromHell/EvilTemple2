import Qt 4.7

MovableWindow {
    width: 400
    height: 480
    title: 'Animations'

    signal playAnimation(string name);

    ListModel {
        id: listModel
    }

    Component {
        id: itemDelegate
        MouseArea {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 25
            Text {
               anchors.left: parent.left
               anchors.right: parent.right
               text: name
               font.pointSize: 12
               font.family: "Fontin"
               font.bold: true
               wrapMode: Text.WrapAtWordBoundaryOrAnywhere
               color: '#FFFFFF'
            }

            onClicked: playAnimation(name)
        }
    }

    ListView {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 6
        anchors.topMargin: 45
        model: listModel
        delegate: itemDelegate
        clip: true
    }

    function setAnimations(animations) {
        listModel.clear();
        for (var i = 0; i < animations.length; ++i) {
            listModel.append({
                name: animations[i]
            });
        }
    }

}
