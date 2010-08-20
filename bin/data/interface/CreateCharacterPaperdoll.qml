import Qt 4.7

import 'CreateCharacterPaperdoll.js' as CreateCharacterPaperdoll
import 'Utilities.js' as Utilities

Rectangle {
    width: 188
    height: 179
    clip: true

    property variant sheet;

    onSheetChanged: CreateCharacterPaperdoll.setCharacter(sheet);

    // Enable this for debugging
    /*Component.onCompleted: {
        sheet = {
            experience: 0,
            level: 10,
            strength: 16,
            dexterity: 13,
            intelligence: 8,
            wisdom: 3,
            constitution: 19,
            charisma: 18,
            hitPoints: 128,
            armorClass: 19,
            fortitudeSave: 5,
            reflexSave: 8,
            willSave: 1,
            initiative: 10,
            speed: 30,
            meleeBonus: 10,
            rangedBonus: 15,
            height: 120,
            weight: 150
        };
    }*/

    Image {
        source: '../art/interface/pc_creation/background.png'
        x: -21
        y: -264
    }

    Image {
        id: grayOutExp
        x: 4
        y: 4
        width: 101
        height: 16
        source: "../art/interface/pc_creation/Gray_States_EXP.png"
    }

    Image {
        id: grayOutHp
        x: 113
        y: 27
        width: 70
        height: 16
        source: "../art/interface/pc_creation/Gray_States_HP.png"
    }

    Image {
        id: grayOutAc
        x: 113
        y: 42
        width: 70
        height: 16
        source: "../art/interface/pc_creation/Gray_States_HP.png"
    }

    Image {
        id: grayOutInit
        x: 4
        y: 119
        width: 89
        height: 16
        source: "../art/interface/pc_creation/Gray_States_INITIATIVE.png"
    }

    Image {
        id: grayOutSpeed
        x: 4
        y: 134
        width: 89
        height: 16
        source: "../art/interface/pc_creation/Gray_States_INITIATIVE.png"
    }

    Image {
        id: grayOutRanged
        x: 95
        y: 134
        width: 89
        height: 16
        source: "../art/interface/pc_creation/Gray_States_INITIATIVE.png"
    }

    Image {
        id: grayOutMelee
        x: 95
        y: 119
        width: 89
        height: 16
        source: "../art/interface/pc_creation/Gray_States_INITIATIVE.png"
    }

    Image {
        id: grayOutFort
        x: 113
        y: 66
        width: 70
        height: 16
        source: "../art/interface/pc_creation/Gray_States_LEVEL.png"
    }


    Image {
        id: grayOutWill
        x: 113
        y: 96
        width: 70
        height: 16
        source: "../art/interface/pc_creation/Gray_States_LEVEL.png"
    }


    Image {
        id: grayOutRef
        x: 113
        y: 81
        width: 70
        height: 16
        source: "../art/interface/pc_creation/Gray_States_LEVEL.png"
    }

    Image {
        id: grayOutStr
        x: 4
        y: 24
        width: 101
        height: 16
        source: "../art/interface/pc_creation/Gray_States_STR.png"
    }

    Image {
        id: grayOutDex
        x: 4
        y: 39
        width: 101
        height: 16
        source: "../art/interface/pc_creation/Gray_States_STR.png"
    }

    Image {
        id: grayOutCon
        x: 4
        y: 54
        width: 101
        height: 16
        source: "../art/interface/pc_creation/Gray_States_STR.png"
    }

    Image {
        id: grayOutInt
        x: 4
        y: 69
        width: 101
        height: 16
        source: "../art/interface/pc_creation/Gray_States_STR.png"
    }

    Image {
        id: grayOutWis
        x: 4
        y: 84
        width: 101
        height: 16
        source: "../art/interface/pc_creation/Gray_States_STR.png"
    }

    Image {
        id: grayOutCha
        x: 4
        y: 99
        width: 101
        height: 16
        source: "../art/interface/pc_creation/Gray_States_STR.png"
    }

    Image {
        id: grayOutHeight
        x: 4
        y: 154
        width: 89
        height: 16
        source: "../art/interface/pc_creation/Gray_States_WT.png"
    }

    Image {
        id: grayOutWeight
        x: 95
        y: 154
        width: 89
        height: 16
        source: "../art/interface/pc_creation/Gray_States_WT.png"
    }

    Image {
        id: grayOutLevel
        x: 113
        y: 4
        width: 70
        height: 16
        source: "../art/interface/pc_creation/Gray_States_FORT.png"
    }

    Column {
        x: 4
        y: 24
        Repeater {
            model: ['STR', 'DEX', 'CON', 'INT', 'WIS', 'CHA']
            Text {
                width: 39
                height: 15
                text: modelData
                color: '#FFFFFF'
                font.bold: true
                horizontalAlignment: "AlignHCenter"
                verticalAlignment: "AlignVCenter"
                visible: !grayOutStr.visible
            }
        }
    }

    Text {
        x: 4
        y: 119
        width: 58
        height: 16
        color: "#ffffff"
        text: "INIT"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutInit.visible
    }

    Text {
        x: 4
        y: 135
        width: 58
        height: 15
        color: "#ffffff"
        text: "SPEED"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutSpeed.visible
    }

    Text {
        x: 95
        y: 119
        width: 58
        height: 16
        color: "#ffffff"
        text: "MELEE"
        verticalAlignment: "AlignVCenter"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutMelee.visible
    }

    Text {
        x: 95
        y: 134
        width: 58
        height: 16
        color: "#ffffff"
        text: "RANGED"
        verticalAlignment: "AlignVCenter"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutRanged.visible
    }

    Text {
        id: text1
        x: 4
        y: 154
        width: 49
        height: 16
        color: "#ffffff"
        text: "Height"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutHeight.visible
    }

    Text {
        id: text2
        x: 95
        y: 154
        width: 49
        height: 16
        color: "#ffffff"
        text: "Weight"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutWeight.visible
    }

    Text {
        x: 113
        y: 66
        width: 38
        height: 15
        color: "#ffffff"
        text: "FORT"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutFort.visible
    }

    Text {
        x: 113
        y: 81
        width: 38
        height: 15
        color: "#ffffff"
        text: "REF"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutRef.visible
    }

    Text {
        x: 113
        y: 97
        width: 38
        height: 15
        color: "#ffffff"
        text: "WILL"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutWill.visible
    }

    Text {
        x: 113
        y: 27
        width: 25
        height: 16
        color: "#ffffff"
        text: "HP"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutHp.visible
    }

    Text {
        x: 113
        y: 42
        width: 25
        height: 16
        color: "#ffffff"
        text: "AC"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutAc.visible
    }

    Text {
        x: 113
        y: 4
        width: 39
        height: 16
        color: "#ffffff"
        text: "LEVEL"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutLevel.visible
    }

    Text {
        x: 4
        y: 4
        width: 39
        height: 16
        color: "#ffffff"
        text: "EXP"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutExp.visible
    }

    Text {
        id: xpDisplay
        x: 46
        y: 4
        width: 59
        height: 16
        color: "#ffffff"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutExp.visible
    }

    Text {
        id: levelDisplay
        x: 155
        y: 4
        width: 28
        height: 16
        color: "#ffffff"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutLevel.visible
    }

    Text {
        id: heightDisplay
        x: 56
        y: 154
        width: 37
        height: 16
        color: "#ffffff"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutHeight.visible
    }

    Text {
        id: weightDisplay
        x: 147
        y: 154
        width: 37
        height: 16
        color: "#ffffff"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutWeight.visible
    }

    Text {
        id: hpDisplay
        x: 142
        y: 27
        width: 41
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutHp.visible
    }

    Text {
        id: fortDisplay
        x: 155
        y: 66
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutFort.visible
    }

    Text {
        id: refDisplay
        x: 155
        y: 81
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutRef.visible
    }

    Text {
        id: willDisplay
        x: 155
        y: 96
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutWill.visible
    }

    Text {
        id: acDisplay
        x: 142
        y: 42
        width: 41
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutAc.visible
    }

    Text {
        id: meleeDisplay
        x: 155
        y: 119
        width: 29
        height: 16
        color: "#ffffff"
        text: "0"
        verticalAlignment: "AlignVCenter"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutMelee.visible
    }

    Text {
        id: rangedDisplay
        x: 155
        y: 134
        width: 29
        height: 16
        color: "#ffffff"
        text: "0"
        verticalAlignment: "AlignVCenter"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        visible: !grayOutRanged.visible
    }

    Text {
        id: strDisplay
        x: 46
        y: 24
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutStr.visible
    }

    Text {
        id: strModDisplay
        x: 77
        y: 24
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutStr.visible
    }

    Text {
        id: dexDisplay
        x: 46
        y: 39
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutDex.visible
    }

    Text {
        id: dexModDisplay
        x: 77
        y: 39
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutDex.visible
    }

    Text {
        id: conDisplay
        x: 46
        y: 54
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutCon.visible
    }

    Text {
        id: conModDisplay
        x: 77
        y: 54
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutCon.visible
    }

    Text {
        id: intDisplay
        x: 46
        y: 69
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutInt.visible
    }

    Text {
        id: intModDisplay
        x: 77
        y: 69
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutInt.visible
    }

    Text {
        id: wisDisplay
        x: 46
        y: 84
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutWis.visible
    }

    Text {
        id: wisModDisplay
        x: 77
        y: 84
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutWis.visible
    }

    Text {
        id: chaDisplay
        x: 46
        y: 99
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutCha.visible
    }

    Text {
        id: chaModDisplay
        x: 77
        y: 99
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutCha.visible
    }

    Text {
        id: initDisplay
        x: 65
        y: 119
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutInit.visible
    }

    Text {
        id: speedDisplay
        x: 65
        y: 134
        width: 28
        height: 16
        color: "#ffffff"
        text: "0"
        font.bold: true
        horizontalAlignment: "AlignHCenter"
        verticalAlignment: "AlignVCenter"
        visible: !grayOutSpeed.visible
    }
}
