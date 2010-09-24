import Qt 4.7
import 'CreateCharacterDomains.js' as CreateCharacterDomains

/*
    Allows the character to choose one or more domains.
*/

Rectangle {
    anchors.fill: parent
    color: '#000000'

    /**
        How many domains may the player select.
      */
    property int numberOfSelectableDomains : 2

    /**
        The list of domains to choose from.
      */
    property variant availableDomains : [
        { id: 'magic', name: 'Magic' },
        { id: 'spell', name: 'Spell' },
        { id: 'plant', name: 'Plant' }
    ]

    signal domainsSelected(variant domains);

    property bool maySelectMore : true

    onAvailableDomainsChanged: {
        listModel.clear();
        availableDomains.forEach(function (domain) {
            listModel.append({
                id: domain.id,
                name: domain.name,
                description: domain.description
            });
        });
    }

    Component {
        id: delegate
        Row {
            spacing: 5
            Checkbox {
                id: checkbox
                anchors.verticalCenter: parent.verticalCenter
                enabled: maySelectMore || checked
                onCheckedChanged: {
                    if (checked)
                        CreateCharacterDomains.selectDomain(model.id)
                    else
                        CreateCharacterDomains.deselectDomain(model.id)
                }
            }

            Text {
                text: model.name
                font.pointSize: 12
                font.family: 'Fontin'
                color: checkbox.enabled ? '#FFFFFF' : '#CCCCCC'
                MouseArea {
                    anchors.fill: parent
                    onClicked: checkbox.toggle()
                }
            }
        }
    }

    ListModel {
        id: listModel
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
            text: "Choose your Character's Domains"
        }

        ScrollView {
            model: listModel
            delegate: delegate
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: headline.bottom
            anchors.topMargin: 10
            spacing: 10
        }

    }
}
