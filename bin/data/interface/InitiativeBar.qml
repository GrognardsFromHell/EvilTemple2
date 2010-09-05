import Qt 4.7

Item {
    id: root

    x: (gameView.viewportSize.width - width) / 2
    y: 15

    width: row.width
    height: row.height

    property variant initiative : [
        {
            id: '{1234-5678-12345}',
            name: 'Storm',
            portrait: 'art/interface/portraits/ELF_1001_m_sorcerer.png',
            initiative: 10,
            active: true
        },
        {
            id: '{1234-5672-12345}',
            name: 'Storm2',
            portrait: 'art/interface/portraits/ELF_1001_m_sorcerer.png',
            initiative: 15
        }
    ]

    Row {
        id: row

        Repeater {
            model: initiative
            Image {
                id: img
                source: '../art/interface/combat_ui/combat_initiative_ui/PortraitFrame_Mini.png'

                Image {
                    x: 3
                    y: 3
                    source: '../' + modelData.portrait
                }

                Image {
                    x: -4
                    y: -4
                    source: '../art/interface/combat_ui/combat_initiative_ui/Highlight_Mini.png'
                    visible: modelData.active == true
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        tooltip.text = "<b>" + modelData.name + "</b><br><i>Initiative: " + modelData.initiative + "</i>";
                        tooltip.shown = true;

                        var pos = root.mapFromItem(img, parent.width / 2, parent.height);

                        tooltip.x = pos.x - tooltip.width / 2;
                        tooltip.y = pos.y;
                    }
                    onExited: tooltip.shown = false;
                }
            }
        }
    }

    Tooltip {
        id: tooltip
    }
}
