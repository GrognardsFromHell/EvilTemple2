import Qt 4.7

Item {
    id: root
    height: childrenRect.height
    y: (gameView.viewportSize.height - height)

    property variant playerCharacters : []
    property variant nonPlayerCharacters : []

    signal action(string type, int index)

    Row {
        spacing: 5

        Row {
            Repeater {
                model: playerCharacters
                PartyPortrait {
                    portrait: modelData.portrait
                    selected: modelData.selected
                    health: modelData.health
                    onClicked: root.action('select', index);
                    onRightClicked: root.action('charsheet', index)
                    onDoubleClicked: root.action('select_and_center', index)
                }
            }
        }

        Rectangle {
            id: spacer
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 20
            color: '#00000000'
            visible: nonPlayerCharacters.length > 0
        }

        Row {
            Repeater {
                model: nonPlayerCharacters
                PartyPortrait {
                    portrait: modelData.portrait
                    selected: modelData.selected
                    health: modelData.health
                    onClicked: root.action('select', 10000 + index);
                    onRightClicked: root.action('charsheet', 10000 + index)
                    onDoubleClicked: root.action('select_and_center', 10000 + index)
                }
            }
        }
    }

}
