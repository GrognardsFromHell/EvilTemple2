import Qt 4.7

Row {
    spacing: 10
    height: 20
    width: childrenRect.width

    property int money : 111

    Row {
        id: goldDisplay
        spacing: 4
        visible: Math.floor(money / 100) > 0
        Text {
            color: '#FFFFFF'
            font.family: "Fontin"
            font.pointSize: 12
            font.weight: Font.Bold
            text: Math.floor(money / 100)
        }
        Image {
            width: 20
            height: 20
            smooth: true
            source: "gold-coin.png"
        }
    }

    Row {
        id: silverDisplay
        spacing: 4
        visible: Math.floor(money / 10) % 10 != 0
        Text {
            color: '#FFFFFF'
            font.family: "Fontin"
            font.pointSize: 12
            font.weight: Font.Bold
            text: Math.floor(money / 10) % 10
        }
        Image {
            width: 20
            height: 20
            smooth: true
            source: "silver-coin.png"
        }
    }


    Row {
        id: copperDisplay
        spacing: 4
        visible: money % 10 != 0
        Text {
            color: '#FFFFFF'
            font.family: "Fontin"
            font.pointSize: 12
            font.weight: Font.Bold
            text: money % 10
        }
        Image {
            width: 20
            height: 20
            smooth: true
            source: "copper-coin.png"
        }
    }
}
