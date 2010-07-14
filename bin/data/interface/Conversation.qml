import Qt 4.7

Item {
    id: root
    width: 640
    height: answerColumn.y + answerColumn.height + 20

    x: (gameView.viewportSize.width - width) / 2
    y: (gameView.viewportSize.height - height) / 2

    property alias npcName : npcNameLabel.text
    property alias npcText : npcTextLabel.text
    property alias answers : answerRepeater.model;
    property string portrait : 'art/interface/portraits/TempMan.png';

    signal answered(int id);

    Rectangle {
        id: backgroundRect

        radius: 5
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#000000"
            }

            GradientStop {
                position: 1
                color: "#000000"
            }
        }
        opacity: 0.5
        anchors.fill: parent
    }

    Text {
        id: npcNameLabel
        x: 21
        y: 14
        width: 600
        height: 19
        color: "#ffffff"
        smooth: false
        style: "Outline"
        wrapMode: "WordWrap"
        font.bold: true
        font.family: "Fontin"
        font.pointSize: 14
        text: 'NPC Name'
    }

    Rectangle {
        id: portraitRect
        x: 19
        y: 44
        width: 113
        height: 100
        color: "#00000000"
        radius: 5
        clip: true
        Image {
            id: portraitImg
            anchors.fill: parent
            fillMode: "Stretch"
            smooth: true
            source: '../' + portrait
        }
    }

    Rectangle {
        id: npctextbg
        x: 142
        y: 44
        width: 479
        height: 100
        radius: 5
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#555555"
            }

            GradientStop {
                position: 0.99
                color: "#292929"
            }
        }
        opacity: 0.5
    }

    Text {
        id: npcTextLabel
        color: "#ffffff"
        style: "Outline"
        font.bold: true
        font.pointSize: 12
        font.family: "Fontin"
        smooth: false
        wrapMode: "WordWrap"
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        anchors.fill: npctextbg
        text: 'Some text the NPC would say'
    }

    Column {
        id: answerColumn
        x: 19
        y: 159
        width: 602
        height: childrenRect.height
        spacing: 5
        Repeater {
            id: answerRepeater
            ConversationLine {
                width: 602
                text: modelData.text
                onClicked: {
                    console.log("Answered: " + modelData.id);
                    root.answered(modelData.id);
                }
            }
        }
    }
}
