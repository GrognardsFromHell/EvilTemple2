import Qt 4.7

Item {
    width: 16
    height: 16

    property alias enabled : mouseArea.enabled

    property bool checked : false

    function toggle() {
        if (enabled)
            checked = !checked;
    }

    Image {
        anchors.fill: parent
        source: parent.enabled ? "checkboxbg.png" : "checkboxbg_disabled.png"
    }

    Image {
        id: checkMark
        source: 'CheckboxMark.png'
        anchors.fill: parent
        smooth: true
        anchors.margins: 3
        visible: checked
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: toggle()
    }
}
