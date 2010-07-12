import Qt 4.7

Item {
    id: root
    width: 640
    height: 380

    x: (gameView.viewportSize.width - width) / 2
    y: (gameView.viewportSize.height - height) / 2

    property alias text : npcText.text;
    property alias answers : answerRepeater.model;
    property string portrait;

    signal answered(int id);

    Rectangle {
        id: backgroundRect

        radius: 5
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#ffffff"
            }

            GradientStop {
                position: 1
                color: "#000000"
            }
        }
        opacity: 0.5
        anchors.fill: parent
    }

    Rectangle {
        id: portraitRect
        x: 19
        y: 24
        width: 100
        height: 100
        radius: 5
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#ffffff"
            }

            GradientStop {
                position: 0.46
                color: "#888888"
            }
        }

        Image {
            id: portraitImg
            anchors.rightMargin: 5
            anchors.leftMargin: 5
            anchors.bottomMargin: 5
            anchors.topMargin: 5
            anchors.fill: parent
            fillMode: "PreserveAspectFit"
            smooth: true
            source: '../' + portrait
        }
    }

    Rectangle {
        x: 131
        y: 24
        width: 490
        height: 100
        radius: 5
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#ffffff"
            }

            GradientStop {
                position: 0.43
                color: "#868686"
            }
        }

        Text {
            id: npcText
            color: "#ffffff"
            text: "text"
            style: "Outline"
            font.bold: true
            font.pointSize: 12
            font.family: "Fontin"
            smooth: true
            wrapMode: "WordWrap"
            anchors.rightMargin: 10
            anchors.leftMargin: 10
            anchors.bottomMargin: 10
            anchors.topMargin: 10
            anchors.fill: parent
        }
    }

    Column {
        x: 19
        y: 129
        width: 602
        height: 232
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
