import Qt 4.7

/*
    The root rectangle will be resized by the view to the maximum viewport size.
*/
Rectangle {
    id: rootRectangle
    color: "black"
    anchors.fill: parent
    width: 800
    height: 600

    signal newGameClicked;

    // Immediately switch to shown state to trigger fade-in transition defined below
    Component.onCompleted: {
        console.log("Main Menu showing");
        state = 'shown'
    }

    Rectangle {
        id: menuBox
        width: 800
        height: 600
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        opacity: 0
        color: "transparent"

        Image {
            id: logo
            width: 403
            height: 268
            anchors.horizontalCenter: parent.horizontalCenter
            y: 6
            fillMode: "PreserveAspectCrop"
            source: "../art/interface/mainmenu_ui/MainMenu_Title.png"
        }

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 354

            MainMenuButton {
                id: newGameButton
                anchors.horizontalCenter: parent.horizontalCenter
                text: 'New Game'
                onClicked: {
                    newGameClicked();
                }
            }

            MainMenuButton {
                id: loadGameButton
                anchors.horizontalCenter: parent.horizontalCenter
                text: 'Load Game'
                onClicked: otherScreen.source = 'LoadGame.qml'
            }

            MainMenuButton {
                id: exitButton
                anchors.horizontalCenter: parent.horizontalCenter
                text: 'Exit Game'
            }
        }
    }

    function backToMainMenu() {
        state = 'shown';
    }

    Loader {
        id: otherScreen
        anchors.fill: parent
        onStatusChanged: if (status == Loader.Ready) {
            console.log("Other screen was loaded: " + item);
            console.log(item.canceled);
            item.canceled.connect(backToMainMenu);
        }
    }

    Image {
        id: splashScreenLogo
        x: 0
        y: 0
        width: 800
        height: 600
        opacity: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        source: "../art/splash/legal0322.png"
        MouseArea {
            anchors.fill: parent
            onClicked: rootRectangle.state = 'shown'
        }
    }

    states: [
        State {
            name: "shown"

            PropertyChanges {
                target: menuBox
                opacity: 1
            }

        },
        State {
            name: "splashScreen"

            PropertyChanges {
                target: splashScreenLogo
                opacity: 1
            }
        },
        State {
            name: "otherScreen"
            when: otherScreen.state == Loader.Ready

            PropertyChanges {
                target: otherScreen
                opacity: 1
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                properties: "opacity"
                duration: 1000
            }
        }
    ]
}
