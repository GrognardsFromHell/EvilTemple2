import Qt 4.7
import EvilTemple 1.0

import 'CreateCharacter.js' as CreateCharacter

Item {
    id: root
    width: 790
    height: 499

    x: (gameView.viewportSize.width - width) / 2
    y: (gameView.viewportSize.height - height) / 2

    Component.onCompleted: CreateCharacter.initialize()

    /**
        Aliasing a property didn't seem to work and caused crashes in debug-mode.
        But returning the object from a function seems to be fine.
      */
    function getModelViewer() {
        return modelViewer;
    }

    property int overallStage : CreateCharacter.Stage.Stats

    property int activeStage : CreateCharacter.Stage.Stats

    signal statsDistributed(int strength, int dexterity, int constitution,
                            int intelligence, int wisdom, int charisma)
    signal raceChosen(string race)
    signal genderChosen(string gender)
    signal heightChosen(int height)

    signal activeStageRequested(string stage)

    // The available races
    property alias races : raceGroup.races

    // Minimum height for the current race/gender selection in centimeters
    property int minHeight

    // Maximum height for the current race/gender selection in centimeters
    property int maxHeight

    onOverallStageChanged: CreateCharacter.updateButtonState()

    onActiveStageChanged: CreateCharacter.updateActiveStage()

    Image {
        id: background
        source: '../art/interface/pc_creation/background.png'
        anchors.fill: parent
    }

    ModelViewer {
        id: modelViewer
        x: 49
        y: 56
        width: 132
        height: 152
    }

    CreateCharacterButtonRight {
        id: statsButton
        x: 665
        y: 31
        text: 'STATS'
        onClicked: activeStageRequested(CreateCharacter.Stage.Stats)
    }

    CreateCharacterButtonRight {
        id: raceButton
        x: 665
        y: 61
        text: 'RACE'
        onClicked: activeStageRequested(CreateCharacter.Stage.Race)
    }

    CreateCharacterButtonRight {
        id: genderButton
        x: 665
        y: 91
        text: 'GENDER'
        onClicked: activeStageRequested(CreateCharacter.Stage.Gender)
    }

    CreateCharacterButtonRight {
        id: heightButton
        x: 665
        y: 121
        text: 'HEIGHT'
        onClicked: activeStageRequested(CreateCharacter.Stage.Height)
    }

    CreateCharacterButtonRight {
        id: hairButton
        x: 665
        y: 151
        text: 'HAIR'
        onClicked: activeStageRequested(CreateCharacter.Stage.Hair)
    }

    CreateCharacterButtonRight {
        id: classButton
        x: 665
        y: 181
        text: 'CLASS'
        onClicked: activeStageRequested(CreateCharacter.Stage.Class)
    }

    CreateCharacterButtonRight {
        id: alignmentButton
        x: 665
        y: 211
        text: 'ALIGNMENT'
        onClicked: activeStageRequested(CreateCharacter.Stage.Alignment)
    }

    CreateCharacterButtonRight {
        id: deityButton
        x: 665
        y: 241
        text: 'DEITY'
        onClicked: activeStageRequested(CreateCharacter.Stage.Deity)
    }

    CreateCharacterButtonRight {
        id: featuresButton
        x: 665
        y: 271
        text: 'FEATURES'
        onClicked: activeStageRequested(CreateCharacter.Stage.Features)
    }

    CreateCharacterButtonRight {
        id: featsButton
        x: 665
        y: 301
        text: 'FEATS'
        onClicked: activeStageRequested(CreateCharacter.Stage.Feats)
    }

    CreateCharacterButtonRight {
        id: skillsButton
        x: 665
        y: 331
        text: 'SKILLS'
        onClicked: activeStageRequested(CreateCharacter.Stage.Skills)
    }

    CreateCharacterButtonRight {
        id: spellsButton
        x: 665
        y: 361
        text: 'SPELLS'
        onClicked: activeStageRequested(CreateCharacter.Stage.Spells)
    }

    CreateCharacterButtonRight {
        id: portraitButton
        x: 665
        y: 391
        text: 'PORTRAIT'
        onClicked: activeStageRequested(CreateCharacter.Stage.Portrait)
    }

    CreateCharacterButtonRight {
        id: voiceAndNameButton
        x: 665
        y: 421
        text: 'VOICE / NAME'
        onClicked: activeStageRequested(CreateCharacter.Stage.VoiceAndName)
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

        onStatsChanged: statsDistributed(statsGroup.strength, statsGroup.dexterity, statsGroup.constitution,
                                    statsGroup.intelligence, statsGroup.wisdom, statsGroup.charisma)
    }

    CreateCharacterRace {
        id: raceGroup
        opacity: 0
        x: 220
        y: 51

        onSelectedRaceChanged: raceChosen(selectedRace)
    }

    CreateCharacterGender {
        id: genderGroup
        opacity: 0
        x: 220
        y: 51

        onGenderChanged: genderChosen(gender)
    }

    CreateCharacterHeight {
        id: heightGroup
        opacity: 0
        x: 220
        y: 51

        onChosenHeightChanged: heightChosen(minHeight + chosenHeight * (maxHeight - minHeight))
    }

    Item {
        id: hairGroup
    }

    Item {
        id: classGroup
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
        },
        State {
            name: "height"
            PropertyChanges {
                target: heightButton
                active: true
            }

            PropertyChanges {
                target: heightGroup
                opacity: 1
            }
        },
        State {
            name: "hair"
            PropertyChanges {
                target: hairButton
                active: true
            }

            PropertyChanges {
                target: hairGroup
                opacity: 1
            }
        },
        State {
            name: "class"
            PropertyChanges {
                target: classButton
                active: true
            }

            PropertyChanges {
                target: classGroup
                opacity: 1
            }
        }
    ]

}
