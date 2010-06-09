import Qt 4.7

Row {
    spacing: 15
    height: 32
    width: 500

    property int platinum: 0;
    property int gold: 0;
    property int silver: 0;
    property int copper: 0;

    Row {
        id: platinumDisplay
        spacing: 4
        visible: platinum != 0
        Image {
            width: 32
            height: 32
            smooth: true
            source: "../art/interface/inventory/coins_platinum.png"
        }
        Text {
            font.family: "Handserif"
            font.pointSize: 18
            font.weight: Font.Bold
            text: platinum
        }
    }


    Row {
        id: goldDisplay
        spacing: 4
        visible: gold != 0
        Image {
            width: 32
            height: 32
            smooth: true
            source: "../art/interface/inventory/coins_gold.png"
        }
        Text {
            font.family: "Handserif"
            font.pointSize: 18
            font.weight: Font.Bold
            text: gold
        }
    }


    Row {
        id: silverDisplay
        spacing: 4
        visible: silver != 0
        Image {
            width: 32
            height: 32
            smooth: true
            source: "../art/interface/inventory/coins_silver.png"
        }
        Text {
            font.family: "Handserif"
            font.pointSize: 18
            font.weight: Font.Bold
            text: silver
        }
    }


    Row {
        id: copperDisplay
        spacing: 4
        visible: copper != 0
        Image {
            width: 32
            height: 32
            smooth: true
            source: "../art/interface/inventory/coins_copper.png"
        }
        Text {
            font.family: "Handserif"
            font.pointSize: 18
            font.weight: Font.Bold
            text: copper
        }
    }
}
