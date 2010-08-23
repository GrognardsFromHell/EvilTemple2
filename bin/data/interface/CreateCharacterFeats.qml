import Qt 4.7
import 'CreateCharacterFeats.js' as CreateCharacterFeats
import 'Constants.js' as Constants

/*
    Allows the character to choose one or more domains.
*/

Rectangle {
    color: '#000000'

    /**
        Triggered when the user "selects" a feat by clicking on it. This should
        display some form of help.
    */
    signal helpRequested(string featId)

    /**
        The user requested to add a feat.
      */
    signal featAddRequested(string featId)

    /**
        The player requested to remove a feat from the list of selected feats.

        We use an index here since complex feats (Weapon Focus) may not be
        identifiable by their id alone. The index is into the list of selected feats.
      */
    signal featRemoveRequested(int index)

    /**
      The list of feats to choose from.
      */
    property variant availableFeats: [
        { id: 'acrobatic', name: 'Acrobatic'},
        { id: 'agile', name: 'Agile' },
        { id: 'alertness', name: 'Alertness' },
        { id: 'anima-affinity', name: 'Animal Affinity' },
        { id: 'armor-profiency-light', name: 'Armor Proficiency (light)' },
        { id: 'armor-profiency-medium', name: 'Armor Proficiency (medium)' },
        { id: 'armor-profiency-heavy', name: 'Armor Proficiency (heavy)' }
    ]

    property variant selectedFeats : []

    onAvailableFeatsChanged: {
        listModel.clear();
        availableFeats.forEach(function (feat) {
            listModel.append({
                id: feat.id,
                name: feat.name
            });
        });
    }

    onSelectedFeatsChanged: {
        selectedFeatsModel.clear();
        selectedFeats.forEach(function (feat) {
            selectedFeatsModel.append({
                id: feat.id,
                name: feat.name
            });
        });
    }

    Component {
        id: delegate
        Item {
            id: root
            property bool selected : false

            property bool isRemoveItem : ListView.view.parent == selectedFeatsList

            height: featName.height + 6
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                id: hoverHighlight
                opacity: 0
                anchors.fill: parent
                color: '#00000000'
                border.color: Constants.HighlightColor
                radius: 5
            }

            Rectangle {
                id: selectedHighlight
                anchors.fill: parent
                radius: 5
                opacity: 0.9
                visible: root.ListView.isCurrentItem
                gradient: HighlightGradient {}
            }

            Text {
                id: featName
                y: 3
                anchors.left: parent.left
                anchors.leftMargin: isRemoveItem ? 35 : 5
                anchors.right: parent.right
                text: model.name
                width: parent.width
                font.pointSize: 12
                font.family: 'Fontin'
                font.bold: false
                color: mouseArea.enabled ? '#FFFFFF' : '#CCCCCC'
                wrapMode: "WrapAtWordBoundaryOrAnywhere"
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onDoubleClicked: {
                    if (isRemoveItem)
                        featRemoveRequested(index)
                    else
                        featAddRequested(model.id);
                }
                onClicked: {
                    root.ListView.view.currentIndex = index;
                    helpRequested(model.id)
                }
                onEntered: {
                    var listView = root.ListView.view;
                    var y = listView.parent.y + parent.y - listView.contentY + (root.height - 64) / 2;

                    if (!isRemoveItem) {
                        buyArrowImage.y = y;
                        buyArrowImage.visible = true;
                        sellArrowImage.visible = false;
                    } else {
                        sellArrowImage.y = y;
                        sellArrowImage.visible = true;
                        buyArrowImage.visible = false;
                    }
                }
                onMousePositionChanged: {
                    var listView = root.ListView.view;
                    var y = listView.parent.y + parent.y - listView.contentY + (root.height - 64) / 2;

                    if (!isRemoveItem) {
                        buyArrowImage.y = y;
                        buyArrowImage.visible = true;
                        sellArrowImage.visible = false;
                    } else {
                        sellArrowImage.y = y;
                        sellArrowImage.visible = true;
                        buyArrowImage.visible = false;
                    }
                }
                onExited: {
                    if (!isRemoveItem) {
                        buyArrowImage.visible = false;
                    } else {
                        sellArrowImage.visible = false;
                    }
                }
            }

            states: [
                State {
                    name: "hover"
                    when: mouseArea.containsMouse
                    PropertyChanges {
                        target: hoverHighlight
                        opacity: 1
                    }
                }
            ]

            transitions: [
                Transition {
                    from: "*"
                    to: "*"
                    NumberAnimation { target: hoverHighlight; property: "opacity"; duration: 100 }
                }
            ]

        }
    }

    ListModel {
        id: listModel
    }

    ListModel {
        id: selectedFeatsModel
        ListElement {
            name: 'test'
        }
    }

    Item {
        anchors.fill: parent
        anchors.margins: 10

        Text {
            id: headline
            anchors.horizontalCenter: parent.horizontalCenter
            height: 17
            color: "#ffffff"
            font.pointSize: 12
            font.family: "Fontin"
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
            text: "Choose your Character's Feats"
        }

        Rectangle {
            anchors.fill: availableFeatsList
            radius: 5
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#7f333333"
                }

                GradientStop {
                    position: 1
                    color: "#7f000000"
                }
            }
        }

        Rectangle {
            anchors.fill: selectedFeatsList
            radius: 5
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#7f333333"
                }

                GradientStop {
                    position: 1
                    color: "#7f000000"
                }
            }
        }

        ScrollView {
            id: availableFeatsList
            model: listModel
            delegate: delegate
            anchors.left: parent.left
            width: parent.width / 2 - 5
            anchors.bottom: parent.bottom
            anchors.top: headline.bottom
            anchors.topMargin: 10
            spacing: 10
        }

        ScrollView {
            id: selectedFeatsList
            model: selectedFeatsModel
            delegate: delegate
            anchors.left: availableFeatsList.right
            anchors.leftMargin: 5
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: headline.bottom
            anchors.topMargin: 10
            spacing: 10
        }

        Image {
            id: buyArrowImage
            visible: false
            source: 'buyarrow_over.png'
            x: (parent.width - width) / 2 - 10
            width: 64
            height: 64
            smooth: true
            opacity: 0.75
            z: 1000
        }

        Image {
            id: sellArrowImage
            visible: false
            source: 'sellarrow_over.png'
            x: (parent.width - width) / 2 - 10
            width: 64
            height: 64
            smooth: true
            opacity: 0.75
            z: 1000
        }

    }
}
