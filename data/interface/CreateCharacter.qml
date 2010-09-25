import Qt 4.7
import EvilTemple 1.0

import 'CreateCharacter.js' as CreateCharacter

Item {
    id: root

    width: gameView.viewportSize.width
    height: gameView.viewportSize.height

    Component.onCompleted: CreateCharacter.initialize()

    /**
        Aliasing a property didn't seem to work and caused crashes in debug-mode.
        But returning the object from a function seems to be fine.
      */
    function getModelViewer() {
        return modelViewer;
    }

    function getFeatsDialog() {
        return featsGroup;
    }

    function getSkillsDialog() {
        return skillsGroup;
    }

    function getPortraitDialog() {
        return portraitGroup;
    }

    function getVoiceAndNameDialog() {
        return voiceAndNameGroup;
    }

    property int overallStage : CreateCharacter.Stage.Stats

    property int activeStage : CreateCharacter.Stage.Stats

    property string largePortrait

    signal statsDistributed(int strength, int dexterity, int constitution,
                            int intelligence, int wisdom, int charisma)
    signal raceChosen(string race)
    signal genderChosen(string gender)
    signal heightChosen(real height)
    signal classChosen(string className)
    signal alignmentChosen(string alignment)
    signal deityChosen(string deity)

    signal activeStageRequested(string stage)

    signal cancel

    // The available races
    property alias races : raceGroup.races

    property alias minHeight : heightGroup.minHeight

    property alias maxHeight : heightGroup.maxHeight

    property alias classes : classGroup.classes

    property alias characterSheet : paperdoll.sheet

    property alias availableAlignments : alignmentGroup.availableAlignments

    property alias availableDeities : deityGroup.availableDeities

    onOverallStageChanged: CreateCharacter.updateButtonState()

    onActiveStageChanged: CreateCharacter.updateActiveStage()

    function loadFeaturesPage(url) {
        featuresLoader.source = url;
        return featuresLoader.item;
    }

    Button {
        x: parent.width - width - 20
        y: parent.height - height - 20
        normalImage: 'art/interface/party_pool/cancel_unselected.png'
        hoverImage: 'art/interface/party_pool/cancel_selected.png'
        pressedImage: 'art/interface/party_pool/cancel_press.png'
        onClicked: cancel()
        text: 'Cancel'
    }

    Item {
        width: 790
        height: 499

        x: (gameView.viewportSize.width - width) / 2
        y: (gameView.viewportSize.height - height) / 2

        Image {
            id: background
            source: '../art/interface/pc_creation/background.png'
            anchors.fill: parent
        }

        CreateCharacterPaperdoll {
            id: paperdoll
            x: 21
            y: 264
        }

        ModelViewer {
            id: modelViewer
            x: 49
            y: 56
            width: 132
            height: 152
            visible: activeStage < CreateCharacter.Stage.Portrait
        }

        Image {
            id: largePortraitImage
            x: 49
            y: 56
            width: 132
            height: 152
            source: '../' + largePortrait
            visible: largePortrait != '' && activeStage >= CreateCharacter.Stage.Portrait
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
            onClicked: activeStageRequested(CreateCharacter.Stage.Finished)
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

            onChosenHeightChanged: heightChosen(chosenHeight)
        }

        Item {
            id: hairGroup
        }

        CreateCharacterClass {
            id: classGroup
            opacity: 0
            x: 220
            y: 51

            onSelectedClassChanged: classChosen(selectedClass)
        }

        CreateCharacterAlignment {
            id: alignmentGroup
            opacity: 0
            x: 220
            y: 51

            onSelectedAlignmentChanged: alignmentChosen(selectedAlignment)
        }

        CreateCharacterDeity {
            id: deityGroup
            opacity: 0
            x: 220
            y: 51

            onSelectedDeityChanged: deityChosen(selectedDeity)
        }

        Loader {
            id: featuresLoader
            opacity: 0
            x: 220
            y: 51
            width: 431
            height: 233
        }

        CreateCharacterFeats {
            id: featsGroup
            opacity: 0
            x: 220
            y: 50
            width: 431
            height: 234
        }

        CreateCharacterSkills {
            id: skillsGroup
            opacity: 0
            x: 220
            y: 50
            width: 431
            height: 234
        }

        CreateCharacterPortrait {
            id: portraitGroup
            opacity: 0
            x: 220
            y: 50
            width: 431
            height: 234
        }

        CreateCharacterVoiceAndName {
            id: voiceAndNameGroup
            opacity: 0
            x: 220
            y: 50
            width: 431
            height: 234
        }
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
        },
        State {
            name: "alignment"
            PropertyChanges {
                target: alignmentButton
                active: true
            }

            PropertyChanges {
                target: alignmentGroup
                opacity: 1
            }
        },
        State {
            name: "deity"
            PropertyChanges {
                target: deityButton
                active: true
            }

            PropertyChanges {
                target: deityGroup
                opacity: 1
            }
        },
        State {
            name: "features"
            PropertyChanges {
                target: featuresButton
                active: true
            }

            PropertyChanges {
                target: featuresLoader
                opacity: 1
            }
        },
        State {
            name: "feats"
            PropertyChanges {
                target: featsButton
                active: true
            }

            PropertyChanges {
                target: featsGroup
                opacity: 1
            }
        },
        State {
            name: "skills"
            PropertyChanges {
                target: skillsButton
                active: true
            }

            PropertyChanges {
                target: skillsGroup
                opacity: 1
            }
        },
        State {
            name: "portrait"
            PropertyChanges {
                target: portraitButton
                active: true
            }

            PropertyChanges {
                target: portraitGroup
                opacity: 1
            }
        },
        State {
            name: "voiceAndName"
            PropertyChanges {
                target: voiceAndNameButton
                active: true
            }

            PropertyChanges {
                target: voiceAndNameGroup
                opacity: 1
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            NumberAnimation { properties: "opacity"; duration: 200 }
        }

    ]

}
