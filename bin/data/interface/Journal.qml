import Qt 4.7

MovableWindow {
    id: root
    width: 750
    height: 420
    title: ''

    Image {
        source: "../art/interface/LOGBOOK_UI/whole_book.png"
        anchors.fill: parent
    }

    Text {
        id: text1
        x: 66
        y: 64
        width: 279
        height: 20
        color: "#ffffff"
        text: "Quests"
        horizontalAlignment: "AlignHCenter"
        smooth: false
        font.pointSize: 12
        font.bold: true
        font.family: "Fontin"
    }

    ListView {
        id: questListView
        x: 66
        y: 84
        width: 279
        height: 292
    }

    Button {
        id: closeButton
        x: 693
        y: 366
        width: 53
        height: 50
        disabledImage: "art/interface/LOGBOOK_UI/exit_normal.png"
        pressedImage: "art/interface/LOGBOOK_UI/exit_click.png"
        normalImage: "art/interface/LOGBOOK_UI/exit_normal.png"
        hoverImage: "art/interface/LOGBOOK_UI/exit_hover.png"
        text: ''
        onClicked: root.closeClicked()
    }
}
