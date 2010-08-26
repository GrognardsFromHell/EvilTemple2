/*****************************************************************************
 *
 * A dialog that allows the player to select an argument for a feat.
 * An example for this is the Weapon Focus feat. It requires the player
 * to specify which type of weapon the feat is for. This dialog accomplishes
 * that.
 *
 *****************************************************************************/

import Qt 4.7

Item {
    width: 291
    height: 357

    x: (gameView.viewportSize.width - width) / 2
    y: (gameView.viewportSize.height - height) / 2

    property alias headline : headline.text

    property variant availableOptions : [
        { id: 'longsword', text: 'Long Sword', requires: 'Str 25', disabled: true },
        { id: 'longsword', text: 'Long Sword' },
        { id: 'longsword', text: 'Long Sword' },
        { id: 'longsword', text: 'Long Sword' },
        { id: 'longsword', text: 'Long Sword' },
        { id: 'longsword', text: 'Long Sword' },
        { id: 'longsword', text: 'Long Sword' },
        { id: 'longsword', text: 'Long Sword' },
        { id: 'longsword', text: 'Long Sword' },
        { id: 'longsword', text: 'Long Sword' },
        { id: 'longsword', text: 'Long Sword' },
        { id: 'longsword', text: 'Long Sword' }
    ]

    signal accepted(string chosenId)
    signal cancelled()

    onAvailableOptionsChanged: {
        selectionModel.clear();
        availableOptions.forEach(function (item) {
            selectionModel.append({
                id: item.id,
                text: item.text,
                requires: item.requires,
                disabled: item.disabled
            });
        });
    }

    Image {
        id: backdrop
        source: '../art/interface/pc_creation/meta_backdrop.png'
        anchors.fill: parent
    }

    Component {
        id: textDelegate
        Item {
            id: root
            anchors.left: parent.left
            anchors.right: parent.right
            height: label.height + requiresLabel.height + 10

            Rectangle {
                id: selectedHighlight
                anchors.fill: parent
                radius: 3
                opacity: 0.9
                visible: root.ListView.isCurrentItem
                gradient: HighlightGradient {}
            }

            StandardText {
                y: 3
                id: label
                text: model.text
                anchors.margins: 3
                anchors.left: parent.left
                anchors.right: parent.right
                wrapMode: "WrapAtWordBoundaryOrAnywhere"
                color: model.disabled ? '#cccccc' : '#ffffff'
            }

            StandardText {
                y: label.y + label.height
                id: requiresLabel
                text: 'Prerequesite: ' + model.requires
                font.italic: true
                font.pointSize: 10
                color: model.disabled ? '#bbbbbb' : '#eeeeee'
                anchors.margins: 3
                anchors.left: parent.left
                anchors.right: parent.right
                wrapMode: "WrapAtWordBoundaryOrAnywhere"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.ListView.view.currentIndex = index
                onDoubleClicked: if (!model.disabled) {
                    accepted(availableOptions[selectionView.currentIndex].id)
                }
            }

            Component.onCompleted: {
                if (!model.requires) {
                    requiresLabel.visible = false;
                    requiresLabel.height = 0;
                }
            }
        }
    }

    ListModel {
        id: selectionModel
    }

    ScrollView {
        id: selectionView
        x: 26
        y: 74
        width: 239
        height: 213
        model: selectionModel
        delegate: textDelegate
        Keys.onEnterPressed: if (selectionView.currentIndex >= 0
                                 && !availableOptions[selectionView.currentIndex].disabled) {
            accepted(availableOptions[selectionView.currentIndex].id);
        }
    }

    StandardText {
        id: headline
        x: 23
        y: 19
        width: 243
        height: 47
        wrapMode: "WrapAtWordBoundaryOrAnywhere"
    }

    Button {
        id: okButton
        x: 28
        y: 306
        text: "Accept"
        enabled: selectionView.currentIndex >= 0 && !availableOptions[selectionView.currentIndex].disabled
        onClicked: accepted(availableOptions[selectionView.currentIndex].id)
    }

    CancelButton {
        id: cancelButton
        x: 152
        y: 306
        onClicked: cancelled()
    }
}
