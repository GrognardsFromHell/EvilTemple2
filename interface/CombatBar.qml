import Qt 4.7

Item {

    x: 15
    y: (gameView.viewportSize.height - height) / 2

    width: childrenRect.width
    height: childrenRect.height

    Component.onCompleted: state = 'grey'

    property real fillPercentage : 0.89

    property alias enabled : endTurnButton.enabled

    signal endTurn

    Image {
        source: '../art/interface/combat_ui/combatbar.png'
    }

    Item {
        x: 13
        y: 15 + (1 - fillPercentage) * bar.sourceSize.height
        clip: true
        width: bar.width
        height: bar.height
        Image {
            id: bar
            y: - (1 - fillPercentage) * bar.sourceSize.height
            source: '../art/interface/combat_ui/CombatBar_FILL.png'
        }
    }

    Button {
        id: endTurnButton
        x: 0
        y: 194
        normalImage: 'art/interface/combat_ui/Action-End-Turn.png'
        pressedImage: 'art/interface/combat_ui/Action-End-Turn-Click.png'
        hoverImage: 'art/interface/combat_ui/Action-End-Turn-Hover.png'
        disabledImage: 'art/interface/combat_ui/Action-End-Turn-Disabled.png'
        text: ''
        onClicked: endTurn()
    }

    Tooltip {
        id: endTurnTooltip
        text: 'End Turn'
        anchors.verticalCenter: endTurnButton.verticalCenter
        anchors.left: endTurnButton.right
        shown: endTurnButton.containsMouse
    }

    states: [
        State {
            name: "gray"
            PropertyChanges {
                target: bar
                source: '../art/interface/combat_ui/CombatBar_GREY.png'
            }
        },
        State {
            name: "highlight"
            PropertyChanges {
                target: bar
                source: '../art/interface/combat_ui/CombatBar_Highlight.png'
            }
        },
        State {
            name: "invalid"
            PropertyChanges {
                target: bar
                source: '../art/interface/combat_ui/CombatBar_Fill_INVALID.png'
            }
        }
    ]

}
