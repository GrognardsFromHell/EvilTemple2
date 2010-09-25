import Qt 4.7

Button {
    id: root
    property bool compatible : false
    property bool selected : false

    normalImage: 'art/interface/pc_creation/alignment_button_normal.png'
    hoverImage: 'art/interface/pc_creation/alignment_button_hovered.png'
    pressedImage: 'art/interface/pc_creation/alignment_button_pressed.png'
    disabledImage: 'art/interface/pc_creation/alignment_button_disabled.png'

    fontSize: 12

    Image {
        source: '../art/interface/pc_creation/alignment_selected.png'
        x: -7
        y: -7
        visible: selected
    }

    states: [
        State {
            name: "active"
            when: compatible || selected
            PropertyChanges {
                target: root
                normalImage: 'art/interface/pc_creation/alignment_button_active.png'
            }
        }
    ]

}
