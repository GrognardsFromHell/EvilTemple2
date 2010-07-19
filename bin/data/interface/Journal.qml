import Qt 4.7

import 'Tests.js' as Tests

MovableWindow {
    id: root
    width: 750
    height: 420
    title: ''

    property variant quests

    onQuestsChanged: {
        questListModel.clear();
        for (var questId in quests) {
            questListModel.append({
                'questId': questId,
                'questState': quests[questId]
            });
        }
    }

    Image {
        source: "../art/interface/LOGBOOK_UI/whole_book.png"
        anchors.fill: parent
    }

    Text {
        id: questsHeadline
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

    ListModel {
        id: questListModel
    }

    Component {
        id: questDelegate
        Text {
            text: questId + ': ' + questState
            color: '#ffffff'
        }
    }

    ListView {
        id: questListView
        x: 66
        y: 84
        width: 271
        height: 290
        model: questListModel
        delegate: questDelegate
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

    Component.onCompleted: Tests.fillQuests(root)
}
