import Qt 4.7

import 'Tests.js' as Tests

MovableWindow {
    id: root
    width: 750
    height: 420
    title: ''

    property variant quests

    property variant reputations

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

    onReputationsChanged: {
        reputationListModel.clear();
        for (var i = 0; i < reputations.length; ++i) {
            var reputation = reputations[i];

            reputationListModel.append({
                'reputationId': reputation.id,
                'reputationName': reputation.name,
                'reputationDescription': reputation.description,
                'reputationEffect': reputation.effect
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

    ListModel {
        id: reputationListModel
    }

    Component {
        id: reputationDelegate
        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            Text {
                text: reputationName
                color: '#ffffff'
                font.family: 'Handserif'
                font.pointSize: 14
                font.bold: true
            }
            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                text: reputationDescription
                color: '#ffffff'
                font.family: 'Fontin'
                font.bold: false
                font.pointSize: 12
                wrapMode: "WrapAtWordBoundaryOrAnywhere"
            }
            Item {
                height: 10
                anchors.left: parent.left
                anchors.right: parent.right
            }
            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                text: reputationEffect
                color: '#ffffff'
                font.family: 'Fontin'
                font.italic: true
                font.bold: false
                font.pointSize: 12
                wrapMode: "WrapAtWordBoundaryOrAnywhere"
            }
        }
    }


    ListView {
        id: reputationListView
        x: 405
        y: 84
        width: 279
        height: 290
        model: reputationListModel
        delegate: reputationDelegate
        clip: true
        spacing: 10
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
