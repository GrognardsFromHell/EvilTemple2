import Qt 4.7

import 'CreateCharacter.js' as CreateCharacter

Item {
    id: root
    width: 790
    height: 499

    Component.onCompleted: CreateCharacter.initialize()

    Image {
        id: background
        source: '../art/interface/pc_creation/background.png'
        anchors.fill: parent
    }

    CreateCharacterButtonRight {
        id: statsButton
        x: 665
        y: 31
        text: 'STATS'
        onClicked: root.state = 'stats-roll'
    }

    CreateCharacterButtonRight {
        id: raceButton
        x: 665
        y: 61
        text: 'RACE'
        onClicked: root.state = 'race'
    }

    CreateCharacterButtonRight {
        id: genderButton
        x: 665
        y: 91
        text: 'GENDER'
        onClicked: root.state = 'gender'
    }

    CreateCharacterButtonRight {
        id: heightButton
        x: 665
        y: 121
        text: 'HEIGHT'
    }

    CreateCharacterButtonRight {
        id: hairButton
        x: 665
        y: 151
        text: 'HAIR'
    }

    CreateCharacterButtonRight {
        id: classButton
        x: 665
        y: 181
        text: 'CLASS'
    }

    CreateCharacterButtonRight {
        id: alignmentButton
        x: 665
        y: 211
        text: 'ALIGNMENT'
    }

    CreateCharacterButtonRight {
        id: deityButton
        x: 665
        y: 241
        text: 'DEITY'
    }

    CreateCharacterButtonRight {
        id: featuresButton
        x: 665
        y: 271
        text: 'FEATURES'
    }

    CreateCharacterButtonRight {
        id: featsButton
        x: 665
        y: 301
        text: 'FEATS'
    }

    CreateCharacterButtonRight {
        id: skillsButton
        x: 665
        y: 331
        text: 'SKILLS'
    }

    CreateCharacterButtonRight {
        id: spellsButton
        x: 665
        y: 361
        text: 'SPELLS'
    }

    CreateCharacterButtonRight {
        id: portraitButton
        x: 665
        y: 391
        text: 'PORTRAIT'
    }

    CreateCharacterButtonRight {
        id: voiceAndNameButton
        x: 665
        y: 421
        text: 'VOICE / NAME'
    }

    Button {
        id: finishButton
        x: 665
        y: 461
        text: 'FINISH'
        normalImage: 'art/interface/pc_creation/alignment_button_normal.png'
        disabledImage: 'art/interface/pc_creation/alignment_button_disabled.png'
        pressedImage: 'art/interface/pc_creation/alignment_button_pressed.png'
        hoverImage:  'art/interface/pc_creation/alignment_button_hovered.png'
    }

    CreateCharacterStats {
        id: statsGroup
        x: 220
        y: 51
        opacity: 0

        onCompleteChanged: CreateCharacter.setStageCompleted(CreateCharacter.Stage.Stats, complete);
    }

    CreateCharacterRace {
        id: raceGroup
        opacity: 0
        x: 220
        y: 51

        onCompleteChanged: CreateCharacter.setStageCompleted(CreateCharacter.Stage.Race, complete);
    }

    CreateCharacterGender {
        id: genderGroup
        opacity: 0
        x: 220
        y: 51

        onCompleteChanged: CreateCharacter.setStageCompleted(CreateCharacter.Stage.Gender, complete);
    }

    states: [
        State {
            name: "stats-roll"
            PropertyChanges {
                target: statsButton
                active: true
            }

            PropertyChanges {
                target: statsGroup
                opacity: 1
            }
        },
        State {
            name: "race"
            PropertyChanges {
                target: raceButton
                active: true
            }

            PropertyChanges {
                target: raceGroup
                opacity: 1
            }
        },
        State {
            name: "gender"
            PropertyChanges {
                target: genderButton
                active: true
            }

            PropertyChanges {
                target: genderGroup
                opacity: 1
            }
        }
    ]

}
