import Qt 4.7
import 'Constants.js' as Constants

Rectangle {

    property string name

    onNameChanged: {
        if (nameInput.text != name)
            nameInput.text = name;
    }

    property string selectedVoice

    property variant voices

    signal requestVoiceSample(string voiceId)

    color: '#000000'

    onVoicesChanged: {
        voiceModel.clear();
        voices.forEach(function(voice) {
            voiceModel.append({
                id: voice.id,
                name: voice.name
            });
        });
    }

    Text {
        x: 0
        y: 5
        width: 433
        height: 17
        color: "#ffffff"
        font.pointSize: 12
        font.family: "Fontin"
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        text: "Choose your Character's Voice and Name"
    }

    StandardText {
        text: 'Name:'
        anchors.right: nameInputBox.left
        anchors.rightMargin: 5
        anchors.verticalCenter: nameInputBox.verticalCenter
    }

    Rectangle {
        id: nameInputBox
        x: 64
        y: 38
        width: 330
        height: 24
        border.color: Constants.HighlightColor
        border.width: 1
        color: '#00000000'
        TextInput {
            id: nameInput
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 4
            color: 'white'
            font.family: 'Fontin'
            font.pointSize: 10
            text: name
            onTextChanged: {
                if (name != text)
                    name = text;
            }
        }
    }

    StandardText {
        text: 'Voice:'
        anchors.right: voiceChoiceBox.left
        anchors.rightMargin: 5
        anchors.verticalCenter: voiceChoiceBox.verticalCenter
    }

    ListModel {
        id: voiceModel
    }

    Component.onCompleted: {
        voiceModel.append({
            id: 'voice1',
            name: 'Brooding'
        });
    }

    Rectangle {
        id: voiceChoiceBox

        anchors.left: nameInputBox.left
        anchors.right: nameInputBox.right
        anchors.top: nameInputBox.bottom
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        border.color: Constants.HighlightColor
        border.width: 1
        color: '#00000000'

        ScrollView {
            anchors.fill: parent
            anchors.margins: 5
            model: voiceModel

            highlightFollowsCurrentItem: true
            highlight: Rectangle {
                border.color: '#7fffffff'
                border.width: 1
                color: '#00000000'
            }

            Keys.onReturnPressed: {
                var entry = voiceModel.get(currentIndex);
                requestVoiceSample(entry.id);
                selectedVoice = entry.id;
            }

            delegate: StandardText {
                anchors.left: parent.left
                anchors.leftMargin: 2
                anchors.right: parent.right
                anchors.rightMargin: 2
                wrapMode: "WrapAtWordBoundaryOrAnywhere"
                text: model.name
                color: selectedVoice == model.id ? Constants.HighlightColor : 'white'
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        requestVoiceSample(model.id)
                        selectedVoice = model.id
                    }
                }
            }
        }
    }



}
