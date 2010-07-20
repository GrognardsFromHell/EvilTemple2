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
        for (var i = 0; i < quests.length; ++i) {
            var quest = quests[i];

            questListModel.append({
                'questId': questId,
                'questState': quest.state,
                'questName': quest.name,
                'questDescription': quest.description
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
        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            Text {
                text: questName
                color: '#ffffff'
                font.family: 'Handserif'
                font.pointSize: 14
                font.bold: true
            }
            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                text: questDescription
                color: '#ffffff'
                font.family: 'Fontin'
                font.bold: false
                font.pointSize: 12
                wrapMode: "WrapAtWordBoundaryOrAnywhere"
            }
        }
    }

    function colorFromSection(section) {
        switch (section) {
        case 'mentioned':
                return 'white';
        case 'accepted':
                return 'yellow';
        case 'completed':
                return 'green';
        case 'botched':
                return 'red';
        }
    }

    // The delegate for each section header
    Component {
        id: sectionHeading
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height + 10
            color: colorFromSection(section)

            Text {
                text: section
                font.bold: true
                width: parent.width
                y: 5
                horizontalAlignment: "AlignHCenter"
            }
        }
    }
    ListView {
        id: questListView
        x: 66
        y: 84
        width: 279
        height: 290
        model: questListModel
        delegate: questDelegate
        clip: true
        section.property: 'questState'
        section.delegate: sectionHeading
        section.criteria: ViewSection.FullString
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
