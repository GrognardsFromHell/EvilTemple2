import Qt 4.7

import 'Utilities.js' as Utilities

Rectangle {

    // When being shown as a view, this is not needed.
    //width: gameView.viewportSize.width
    //height: gameView.viewportSize.height

    width: 800
    height: 600
    color: 'black'

    property variant saveGameList

    signal closeDialog
    signal loadGame(string id)

    onSaveGameListChanged: {
        print("Showing " + saveGameList.length + " save games.");
        saveGameModel.clear();
        saveGameList.forEach(function (item) {
            saveGameModel.append({
                saveId: item.id,
                saveName: item.name,
                saveScreenshot: item.screenshot,
                saveCreated: item.created
            });
        });
    }

    Image {
        id: screenshotView
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        opacity: 0.5
    }

    ListModel {
        id: saveGameModel
        ListElement {
            saveId: 'some-save-id'
            saveName: 'Quicksave'
        }
        ListElement {
            saveId: 'some-save-id'
            saveName: 'Quicksave2'
            saveCreated: '12345'
        }
    }

    Component {
        id: saveGameDelegate
        MouseArea {
            id: wrapper
            anchors.left: parent.left
            anchors.right: parent.right
            height: saveNameText.height + 10
            hoverEnabled: true

            Rectangle {
                id: itemBackground
                radius: 5
                anchors.fill: parent
                opacity: wrapper.containsMouse ?  0.5 : 0.25
                gradient: Gradient {
                    GradientStop {
                        position: 0
                        id: stop1
                        color: "#555555"
                    }

                    GradientStop {
                        position: 1
                        id: stop2
                        color: "#292929"
                    }
                }
            }

            Text {
                y: 5
                x: 5
                id: saveNameText
                text: model.saveName
                font.family: 'Fontin'
                font.bold: true
                font.pointSize: 12
                color: '#ffffff'
            }

            Text {
                y: 5
                anchors.right: parent.right
                anchors.rightMargin: 5
                font.family: 'Fontin'
                font.bold: true
                font.pointSize: 12
                color: '#ffffff'
                text: Utilities.getComfortableTime(saveCreated)
            }

            onClicked: {
                var pos = mapToItem(ListView.view, mouseX, mouseY);
                ListView.view.currentIndex = ListView.view.indexAt(pos.x, pos.y);
            }

            onDoubleClicked: {
                var pos = mapToItem(ListView.view, mouseX, mouseY);
                var idx = ListView.view.indexAt(pos.x, pos.y);
                loadGame(saveGameModel.get(idx).saveId);
            }

            states: [
                State {
                    name: "ActiveState"
                    when: wrapper.ListView.isCurrentItem

                    PropertyChanges { target: itemBackground; opacity: 1 }
                    PropertyChanges { target: stop1; color: '#00b7e8' }
                    PropertyChanges { target: stop2; color: '#005993' }
                }
            ]

            transitions: [
                Transition {
                    from: "*"
                    to: "*"
                    NumberAnimation { properties: "opacity"; duration: 200 }
                }
            ]

        }
    }

    Item {

        anchors.centerIn: parent
        width: 500
        height: 400

        Rectangle {
            radius: 5
            opacity: 0.5
            anchors.fill: parent
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#333333"
                }

                GradientStop {
                    position: 1
                    color: "#000000"
                }
            }
        }

        ListView {
            id: saveGameListView
            width: 414
            height: 268
            delegate: saveGameDelegate
            model: saveGameModel
            spacing: 10

            clip: true

            anchors.left: parent.left
            anchors.leftMargin: 15

            anchors.top: parent.top
            anchors.topMargin: 15

            anchors.right: parent.right
            anchors.rightMargin: 15

            anchors.bottom: loadGameButton.top
            anchors.bottomMargin: 15

            focus: true

            Keys.onDownPressed: {
                incrementCurrentIndex()
            }

            Keys.onUpPressed: {
                decrementCurrentIndex()
            }

            Keys.onEnterPressed: {
                loadGame(model.get(currentIndex).saveId)
            }

            onCurrentIndexChanged: {
                var modelItem = model.get(currentIndex);
                screenshotView.source = modelItem.saveScreenshot;
            }
        }

        Image {
            id: loadGameButton
            source: 'ButtonBackdrop.png'
            width: loadGameButtonInternal.width + 8
            height: loadGameButtonInternal.height + 8
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 15
            anchors.left: parent.left
            anchors.leftMargin: 15
            Button {
                x: 4
                y: 4
                id: loadGameButtonInternal
                text: 'Load Game'
                enabled: saveGameListView.currentItem != null
                onClicked: loadGame(saveGameModel.get(saveGameListView.currentIndex).saveId)
            }
        }

        Image {
            id: cancelButton
            source: 'ButtonBackdrop.png'
            width: cancelButtonInternal.width + 8
            height: cancelButtonInternal.height + 8
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 15
            anchors.left: loadGameButton.right
            anchors.leftMargin: 15
            Button {
                x: 4
                y: 4
                id: cancelButtonInternal
                text: 'Cancel'
                onClicked: closeDialog()
            }
        }
    }
}
