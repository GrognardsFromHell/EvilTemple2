import Qt 4.7

import 'ChoosePartyAlignment.js' as ChoosePartyAlignment

Item {
    id: root

    property string activeAlignment : '';

    signal cancelled
    signal alignmentSelected(string alignment)

    width: 483
    height: 402

    Item {
        width: 483
        height: 402
        anchors.centerIn: parent

        Image {
            id: background
            anchors.fill: parent
            source: '../art/interface/pc_creation/party_alignment.png'
        }

        Button {
            id: acceptButton
            x: 105
            y: 354
            normalImage: 'art/interface/pc_creation/button_done_normal.png'
            hoverImage: 'art/interface/pc_creation/button_done_hover.png'
            pressedImage: 'art/interface/pc_creation/button_done_click.png'
            disabledImage: 'art/interface/pc_creation/button_done_disabled.png'
            text: 'Accept'
            enabled: root.activeAlignment != ''
            onClicked: root.alignmentSelected(root.activeAlignment)
        }

        Button {
            id: cancelButton
            x: 266
            y: 354
            normalImage: 'art/interface/pc_creation/cancel_normal.png'
            hoverImage: 'art/interface/pc_creation/cancel_hover.png'
            pressedImage: 'art/interface/pc_creation/cancel_pressed.png'
            text: 'Cancel'
            onClicked: root.cancelled()
        }

        AlignmentButton {
            id: lgButton
            x: 53
            y: 73
            text: 'Lawful Good'
            onClicked: ChoosePartyAlignment.activateAlignment('lg')
            selected: activeAlignment == 'lg'
            compatible: activeAlignment == 'ng' || activeAlignment == 'ln'
        }

        AlignmentButton {
            id: ngButton
            x: 184
            y: 73
            text: 'Neutral Good'
            onClicked: ChoosePartyAlignment.activateAlignment('ng')
            selected: activeAlignment == 'ng'
            compatible: activeAlignment == 'lg' || activeAlignment == 'n' || activeAlignment == 'cg'
        }

        AlignmentButton {
            id: cgButton
            x: 315
            y: 73
            text: 'Chaotic Good'
            onClicked: ChoosePartyAlignment.activateAlignment('cg')
            selected: activeAlignment == 'cg'
            compatible: activeAlignment == 'ng' || activeAlignment == 'cn'
        }

        AlignmentButton {
            id: lnButton
            x: 53
            y: 124
            text: 'Lawful Neutral'
            onClicked: ChoosePartyAlignment.activateAlignment('ln')
            selected: activeAlignment == 'ln'
            compatible: activeAlignment == 'lg' || activeAlignment == 'n' || activeAlignment == 'le'
        }

        AlignmentButton {
            id: nButton
            x: 184
            y: 124
            text: 'True Neutral'
            onClicked: ChoosePartyAlignment.activateAlignment('n')
            selected: activeAlignment == 'n'
            compatible: activeAlignment == 'ng' || activeAlignment == 'ln' || activeAlignment == 'cn' ||
                    activeAlignment == 'ne'
        }

        AlignmentButton {
            id: cnButton
            x: 315
            y: 124
            text: 'Chaotic Neutral'
            onClicked: ChoosePartyAlignment.activateAlignment('cn')
            selected: activeAlignment == 'cn'
            compatible: activeAlignment == 'n' || activeAlignment == 'cg' || activeAlignment == 'ce'
        }

        AlignmentButton {
            id: leButton
            x: 53
            y: 175
            text: 'Lawful Evil'
            onClicked: ChoosePartyAlignment.activateAlignment('le')
            selected: activeAlignment == 'le'
            compatible: activeAlignment == 'ln' || activeAlignment == 'ne'
        }

        AlignmentButton {
            id: neButton
            x: 184
            y: 175
            text: 'Neutral Evil'
            onClicked: ChoosePartyAlignment.activateAlignment('ne')
            selected: activeAlignment == 'ne'
            compatible: activeAlignment == 'le' || activeAlignment == 'n' || activeAlignment == 'ce'
        }

        AlignmentButton {
            id: ceButton
            x: 315
            y: 175
            text: 'Chaotic Evil'
            onClicked: ChoosePartyAlignment.activateAlignment('ce')
            selected: activeAlignment == 'ce'
            compatible: activeAlignment == 'ne' || activeAlignment == 'cn'
        }

        Text {
            id: text1
            x: 0
            y: 8
            width: 483
            height: 20
            color: "#ffffff"
            text: "Party Alignment"
            font.bold: false
            font.pointSize: 13
            font.family: "Handserif"
            horizontalAlignment: "AlignHCenter"
        }

        Text {
            id: text2
            x: 28
            y: 40
            width: 425
            height: 16
            color: "#ffffff"
            text: "Choose your Party's Alignment"
            verticalAlignment: "AlignVCenter"
            font.family: "Fontin"
            font.pointSize: 12
            horizontalAlignment: "AlignLeft"
        }

    }
}
