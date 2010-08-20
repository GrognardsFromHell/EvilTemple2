import Qt 4.7
import 'Alignment.js' as Alignment

Rectangle {
    id: root
    width: 431
    height: 233

    color: '#000000'

    property string selectedAlignment

    property variant availableAlignments : [Alignment.LawfulGood, Alignment.NeutralGood, Alignment.ChaoticGood,
                                            Alignment.LawfulNeutral, Alignment.TrueNeutral, Alignment.ChaoticNeutral,
                                            Alignment.LawfulEvil, Alignment.NeutralEvil, Alignment.ChaoticEvil]

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
        text: "Choose your Character's Alignment"
    }

    Item {
        x: 21
        y: 42
        width: 390
        height: 150
        clip: true
        Image {
            x: -45
            y: -65
            width: sourceSize.width
            height: sourceSize.height
            source: "../art/interface/pc_creation/party_alignment.png"
        }
    }

    AlignmentButton {
        id: lgButton
        x: 29
        y: 50
        text: 'Lawful Good'
        onClicked: selectedAlignment = Alignment.LawfulGood
        selected: selectedAlignment == Alignment.LawfulGood
        enabled: availableAlignments.indexOf(Alignment.LawfulGood) != -1
    }

    AlignmentButton {
        id: ngButton
        x: 160
        y: 50
        text: 'Neutral Good'
        onClicked: selectedAlignment = Alignment.NeutralGood
        selected: selectedAlignment == Alignment.NeutralGood
        enabled: availableAlignments.indexOf(Alignment.NeutralGood) != -1
    }

    AlignmentButton {
        id: cgButton
        x: 291
        y: 50
        text: 'Chaotic Good'
        onClicked: selectedAlignment = Alignment.ChaoticGood
        selected: selectedAlignment == Alignment.ChaoticGood
        enabled: availableAlignments.indexOf(Alignment.ChaoticGood) != -1
    }

    AlignmentButton {
        id: lnButton
        x: 29
        y: 101
        text: 'Lawful Neutral'
        onClicked: selectedAlignment = Alignment.LawfulNeutral
        selected: selectedAlignment == Alignment.LawfulNeutral
        enabled: availableAlignments.indexOf(Alignment.LawfulNeutral) != -1
    }

    AlignmentButton {
        id: nButton
        x: 160
        y: 101
        text: 'True Neutral'
        onClicked: selectedAlignment = Alignment.TrueNeutral
        selected: selectedAlignment == Alignment.TrueNeutral
        enabled: availableAlignments.indexOf(Alignment.TrueNeutral) != -1
    }

    AlignmentButton {
        id: cnButton
        x: 291
        y: 101
        text: 'Chaotic Neutral'
        onClicked: selectedAlignment = Alignment.ChaoticNeutral
        selected: selectedAlignment == Alignment.ChaoticNeutral
        enabled: availableAlignments.indexOf(Alignment.ChaoticNeutral) != -1
    }

    AlignmentButton {
        id: leButton
        x: 29
        y: 152
        text: 'Lawful Evil'
        onClicked: selectedAlignment = Alignment.LawfulEvil
        selected: selectedAlignment == Alignment.LawfulEvil
        enabled: availableAlignments.indexOf(Alignment.LawfulEvil) != -1
    }

    AlignmentButton {
        id: neButton
        x: 160
        y: 152
        text: 'Neutral Evil'
        onClicked: selectedAlignment = Alignment.NeutralEvil
        selected: selectedAlignment == Alignment.NeutralEvil
        enabled: availableAlignments.indexOf(Alignment.NeutralEvil) != -1
    }

    AlignmentButton {
        id: ceButton
        x: 291
        y: 152
        text: 'Chaotic Evil'
        onClicked: selectedAlignment = Alignment.ChaoticEvil
        selected: selectedAlignment == Alignment.ChaoticEvil
        enabled: availableAlignments.indexOf(Alignment.ChaoticEvil) != -1
    }


}
