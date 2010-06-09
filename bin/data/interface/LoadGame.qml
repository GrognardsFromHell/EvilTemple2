import Qt 4.7

Rectangle {
    signal canceled()

    color: "yellow"
    anchors.fill: parent
    width: 640
    height: 480

    MouseArea {
        anchors.fill: parent
        onClicked: parent.canceled()
    }

    Component.onCompleted: {
        var listSaves = savegames.listSaves();
        for (var i = 0; i < listSaves.length; ++i) {
            var save = listSaves[i];
            console.log("found savegame: " + save.name);
        }
    }
}
