import Qt 4.7

Button {
    id: root

    property bool active : false
    property bool done : false

    text: 'Button'

    fontSize: 12
    fontFamily: 'Fontin'

    normalImage: 'art/interface/pc_creation/button_active_normal.png'
    hoverImage: 'art/interface/pc_creation/button_active_hover.png'
    pressedImage: 'art/interface/pc_creation/button_active_click.png'
    disabledImage: 'art/interface/pc_creation/button_disabled_normal.png'

    states: [
        State {
            name: "done-active"
            when: done && active
            PropertyChanges {
                target: root
                normalImage: 'art/interface/pc_creation/button_done_activated.png'
                hoverImage: 'art/interface/pc_creation/button_done_hover.png'
                pressedImage: 'art/interface/pc_creation/button_done_click.png'
                disabledImage: 'art/interface/pc_creation/button_done_normal.png'
            }
        },
        State {
            name: "done"
            when: done
            PropertyChanges {
                target: root
                normalImage: 'art/interface/pc_creation/button_done_normal.png'
                hoverImage: 'art/interface/pc_creation/button_done_hover.png'
                pressedImage: 'art/interface/pc_creation/button_done_click.png'
                disabledImage: 'art/interface/pc_creation/button_done_normal.png'
            }
        },
        State {
            name: "active"
            when: active
            PropertyChanges {
                target: root
                normalImage: 'art/interface/pc_creation/button_active_activated.png'
            }
        }
    ]

}
