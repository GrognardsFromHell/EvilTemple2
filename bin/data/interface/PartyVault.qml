import Qt 4.7

import 'Constants.js' as Constants

Item {
    width: 1024
    height: 768

    signal closeClicked

    property int partySize: 6

    property variant characters : [
        {
            id: '{1234-5678-12345}',
            name: 'Storm',
            portrait: 'art/interface/portraits/ELF_1002_s_sorcerer.png',
            gender: 'Male',
            race: 'Human',
            alignment: 'Lawful Good',
            classes: 'Fighter 1 Wizard 2',
            compatible: true
        },
        {
            id: '{1234-5672-12345}',
            name: 'Storm2',
            portrait: 'art/interface/portraits/ELF_1002_s_sorcerer.png',
            gender: 'Male',
            race: 'Human',
            alignment: 'Lawful Evil',
            classes: 'Cleric 1',
            compatible: false
        }
    ]

    onCharactersChanged: {
        characterModel.clear();
        characters.forEach(function (character) {
            characterModel.append({
                id: character.id,
                name: character.name,
                portrait: character.portrait,
                gender: character.gender,
                race: character.race,
                alignment: character.alignment,
                classes: character.classes,
                compatible: character.compatible
            });
        });
    }

    function updateAddButton() {
        addButton.enabled = true;

        var entry = characterModel.get(charactersView.currentIndex);
        if (!entry.compatible || isSelected(entry.id)) {
            addButton.enabled = false;
            return;
        }
    }

    function isSelected(id) {

        for (var i = 0; i < selectedParty.count; ++i) {
            if (selectedParty.get(i).id == id)
                return true;
        }

        return false;
    }

    function removePartyMember(index) {
        selectedParty.remove(index);
    }

    function addPartyMember(index) {
        var entry = characterModel.get(index)
        selectedParty.append(entry);
    }

    Item {
        width: childrenRect.width
        height: childrenRect.height
        anchors.centerIn: parent

        Image {
            source: '../art/interface/party_pool/background.png'

            // Fills the scroll pane with a black color to remove the pre-drawn boxes
            Rectangle {
                x: 37
                y: 82
                width: 468
                height: 373
                color: "#000000"
            }
        }

        Button {
            id: createButton
            x: 66
            y: 52
            normalImage: 'art/interface/party_pool/normal.png'
            pressedImage: 'art/interface/party_pool/press.png'
            hoverImage: 'art/interface/party_pool/hover.png'
            disabledImage: 'art/interface/party_pool/disable.png'
            text: "Create"
        }

        Button {
            id: deleteButton
            x: 372
            y: 52
            normalImage: 'art/interface/party_pool/normal.png'
            pressedImage: 'art/interface/party_pool/press.png'
            hoverImage: 'art/interface/party_pool/hover.png'
            disabledImage: 'art/interface/party_pool/disable.png'
            text: "Delete"
        }

        Button {
            id: viewButton
            x: 168
            y: 52
            normalImage: 'art/interface/party_pool/normal.png'
            pressedImage: 'art/interface/party_pool/press.png'
            hoverImage: 'art/interface/party_pool/hover.png'
            disabledImage: 'art/interface/party_pool/disable.png'
            text: "View"
        }

        Button {
            id: renameButton
            x: 270
            y: 52
            normalImage: 'art/interface/party_pool/normal.png'
            pressedImage: 'art/interface/party_pool/press.png'
            hoverImage: 'art/interface/party_pool/hover.png'
            disabledImage: 'art/interface/party_pool/disable.png'
            text: "Rename"
        }

        Button {
            id: cancelButton
            x: 714
            y: 428
            width: 53
            height: 50
            disabledImage: "art/interface/CHAR_UI/main_exit_button_disabled.png"
            pressedImage: "art/interface/CHAR_UI/main_exit_button_pressed.png"
            normalImage: "art/interface/CHAR_UI/main_exit_button_hover_off.png"
            hoverImage: "art/interface/CHAR_UI/main_exit_button_hover_on.png"
            text: ''
            onClicked: closeClicked()
        }

        Button {
            id: addButton
            x: 168
            y: 465
            normalImage: 'art/interface/party_pool/add_normal.png'
            pressedImage: 'art/interface/party_pool/add_press.png'
            hoverImage: 'art/interface/party_pool/add_hover.png'
            disabledImage: 'art/interface/party_pool/add_disable.png'
            text: "Add"
            visible: selectedPartyView.selected == -1
        }

        Button {
            id: removeButton
            x: 168
            y: 465
            normalImage: 'art/interface/party_pool/remove_normal.png'
            pressedImage: 'art/interface/party_pool/remove_press.png'
            hoverImage: 'art/interface/party_pool/remove_hover.png'
            disabledImage: 'art/interface/party_pool/remove_disable.png'
            text: "Remove"
            visible: selectedPartyView.selected != -1
            onClicked: removePartyMember(selectedPartyView.selected)
        }

        ListModel {
            id: characterModel
        }

        Component {
            id: characterDelegate
            Row {
                anchors.left: parent.left
                anchors.right: parent.right
                Item {
                    height: infoColumn.height
                    width: infoColumn.height
                    Image {
                        source: '../' + model.portrait
                        anchors.centerIn: parent
                    }
                }
                Column {
                    id: infoColumn
                    StandardText {
                        text: model.name
                        font.bold: true
                    }
                    StandardText {
                        text: model.gender + " " + model.race
                    }
                    StandardText {
                        text: model.classes
                    }
                    StandardText {
                        text: model.alignment
                    }
                }
            }
        }

        ScrollView {
            id: charactersView
            x: 37
            y: 82
            width: 468
            height: 373
            model: characterModel
            delegate: characterDelegate
            highlight: Rectangle {
                radius: 5
                color: Constants.HighlightColor
                opacity: 0.75
            }
        }
    }

    ListModel {
        id: selectedParty
        ListElement {
            portrait: 'art/interface/portraits/ELF_1002_s_sorcerer.png'
        }
    }

    Row {
        id: selectedPartyView
        x: 15
        spacing: 15
        anchors.bottom: parent.bottom

        property int selected : -1

        Repeater {
            model: selectedParty
            Image {
                source: '../art/interface/pc_creation/portrait.png'
                Image {
                    anchors.fill: portrait
                    anchors.margins: -10
                    source: 'ImageGlow.png'
                    visible: selectedPartyView.selected == index;
                }
                Image {
                    id: portrait
                    x: 4
                    y: 4
                    source: '../' + model.portrait
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (selectedPartyView.selected == index) {
                                selectedPartyView.selected = -1;
                            } else {
                                selectedPartyView.selected = index;
                            }
                        }
                    }
                }
            }
        }
    }

    Row {
        id: freePartySlotsRow
        spacing: 15
        anchors.left: selectedPartyView.right
        anchors.leftMargin: 15
        anchors.bottom: parent.bottom
        Repeater {
            model: partySize - selectedParty.count
            Image {
                source: '../art/interface/pc_creation/portrait.png'
            }
        }
    }
}
