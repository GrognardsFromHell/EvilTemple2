import Qt 4.7

MovableWindow {
    width: 640
    height: 480

    title: ""

    property alias text : content.text

    signal linkClicked(string href)

    Flickable {
        clip: true
        anchors.fill: parent
        anchors.topMargin: 25
        anchors.leftMargin: 15
        anchors.rightMargin: 15
        anchors.bottomMargin: 15
        contentWidth: width
        contentHeight: content.height

        Text {
            id: content
            text: "text\ntext\n<a href='#'>text</a>\ntext\ntext\ntext\ntext\ntext\ntext\ntext\n"
                + "text\ntext\ntext\ntext\ntext\ntext\ntext\ntext\ntext\n<img src=\"http://www.co8.org/forum/images/smilies/icon_hug.gif\">"
            font.family: 'Fontin'
            font.pointSize: 12
            color: '#FFFFFF'
            width: parent.width
            wrapMode: "WrapAtWordBoundaryOrAnywhere"
            textFormat: "RichText"
            onLinkActivated: linkClicked(link)
        }
    }
}
