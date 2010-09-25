import Qt 4.7

MouseArea {
    id: root
    height: row.height + 10
    width: row.width + 10

    hoverEnabled: true

    property string iconPath
    property int quantity
    property int location
    property string description
    property bool magical
    property int weight
    property int worth : 0

    states: [
        State {
            name: "hover"
            when: containsMouse
            PropertyChanges { target: stop1; color: '#00b7e8' }
            PropertyChanges { target: stop2; color: '#005993' }
        }
    ]

    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        radius: 5
        gradient: Gradient {
            GradientStop {
                id: stop1
                position: 0
                color: "#555555"
            }

            GradientStop {
                id: stop2
                position: 0.99
                color: "#292929"
            }
        }
        opacity: 0.5
    }

    Row {
        id: row
        y: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 5
        Image {
            source: '../' + root.iconPath
            width: 40
            height: 40
            smooth: true
        }
        Column {
            spacing: 2
            Text {
                text: description
                font.family: 'Fontin'
                font.pointSize: 12
                font.weight: Font.Bold
                color: magical ? '#00a9f8' : '#ffffff'
            }
            Row {
                spacing: 15
                MoneyDisplay {
                    money: worth
                    visible: worth > 0
                }
                Text {
                    text: "<i>Quantity:</i> " + quantity
                    font.family: 'Fontin'
                    font.pointSize: 12
                    visible: quantity > 1
                    color: '#eeeeee'
                }
                Text {
                    text: "<i>Location:</i> " + location
                    font.family: 'Fontin'
                    font.pointSize: 12
                    color: '#eeeeee'
                    visible: location != 0
                }
                Text {
                    text: "<i>" + weight + " lbs</i>"
                    font.family: 'Fontin'
                    font.pointSize: 12
                    color: '#eeeeee'
                    visible: weight != 0
                }
            }
        }
    }
}
