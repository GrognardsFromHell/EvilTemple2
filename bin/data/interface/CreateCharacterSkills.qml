import Qt 4.7

import 'Constants.js' as Constants

Rectangle {
    width: 640
    height: 480
    color: 'black'

    property int remainingSkillPoints : 4
    property int availableSkillPoints : 4

    property string detailsRank : ''
    property string detailsAbility : ''
    property string detailsAbilityMod : ''
    property string detailsMiscMod : ''
    property string detailsTotalMod : ''

    property variant skillPointDistribution

    signal requestDetails(string skillId, variant currentDistribution)

    property variant skills : [
        { id: 'appraise',
          name: 'Appraise',
          rank: 10,
          classSkill: false,
          minimumRank: 10,
          maximumRank: 12
        },
        { id: 'bluff',
          name: 'Bluff',
          rank: 10,
          classSkill: true,
          minimumRank: 10,
          maximumRank: 12
        },
        { id: 'concentration',
          name: 'Concentration',
          rank: 10,
          classSkill: true,
          minimumRank: 10,
          maximumRank: 12
        },
        { id: 'diplomacy',
          name: 'Diplomacy',
          rank: 10,
          classSkill: true,
          minimumRank: 10,
          maximumRank: 12
        },
        { id: 'disableDevice',
          name: 'Disable Device',
          rank: 10,
          classSkill: true,
          minimumRank: 10,
          maximumRank: 12
        },
        { id: 'gatherinformation',
          name: 'Gather Information',
          rank: 10,
          classSkill: true,
          minimumRank: 10,
          maximumRank: 12
        },
        { id: 'use-mgic-device',
          name: 'Use Magic Device',
          rank: 10,
          classSkill: false,
          minimumRank: 10,
          maximumRank: 12
        }
    ]

    function getCurrentSkillRanks() {
        var result = {};

        for (var i = 0; i < skillModel.count; ++i) {
            var entry = skillModel.get(i);
            result[entry.id] = entry.rank;
        }

        return result;
    }

    function updateDetails(index) {
        if (index == -1) {
            rankLabel.text = '';
            miscLabel.text = '';
            abilityModLabel.text = '';
            totalBonusLabel.text = '';
            abilityTypeLabel.text = '';
        } else {
            var entry = skillModel.get(index);

            requestDetails(entry.id, getCurrentSkillRanks());
        }
    }

    function updateSkillPointDistribution() {
        var result = {};

        for (var i = 0; i < skillModel.count; ++i) {
            var entry = skillModel.get(i);
            var added = entry.rank - entry.minimumRank;
            if (added > 0)
                result[entry.id] = added;
        }

        skillPointDistribution = result;
    }

    function increaseRank(index) {
        var entry = skillModel.get(index);
        if (entry.rank < entry.maximumRank) {
            remainingSkillPoints--;
            if (entry.classSkill) {
                entry.rank += 2;
            } else {
                entry.rank++;
            }
            skillModel.set(index, entry);
        }
        updateDetails(index);
        updateSkillPointDistribution();
    }

    function decreaseRank(index) {
        var entry = skillModel.get(index);
        if (entry.rank > entry.minimumRank) {
            remainingSkillPoints++;
            if (entry.classSkill) {
                entry.rank -= 2;
            } else {
                entry.rank--;
            }
            skillModel.set(index, entry);
        }
        updateDetails(index);
        updateSkillPointDistribution();
    }

    onSkillsChanged: {
        skillModel.clear();

        // Sort according to section, then name
        var sortedSkills = skills.slice(0);
        sortedSkills.sort(function (a, b) {
            if (a.classSkill && !b.classSkill) {
                return -1
            } else if (!a.classSkill && b.classSkill) {
                return 1;
            } else {
                return a.name.localeCompare(b.name);
            }
        });

        sortedSkills.forEach(function (skill) {
            var section = skill.classSkill ? "classSkill" : "crossClassSkill;"

            skillModel.append({
                id: skill.id,
                name: skill.name,
                rank: skill.rank,
                classSkill: skill.classSkill,
                section: section,
                maximumRank: skill.maximumRank,
                minimumRank: skill.minimumRank
            });
        });
    }

    Component {
        id: skillDelegate
        Item {
            id: root
            height: childrenRect.height
            anchors.left: parent.left
            anchors.right: parent.right

            Keys.onPressed: {
                if (event.key == Qt.Key_Plus) {
                    increaseRank(index);
                } else if (event.key == Qt.Key_Minus) {
                    decreaseRank(index);
                }
            }

            StandardText {
                anchors.left: parent.left
                anchors.right: spinnerColumn.left
                anchors.rightMargin: 5
                anchors.verticalCenter: rankBox.verticalCenter
                horizontalAlignment: "AlignRight"
                text: model.name
                font.pointSize: 10
                color: (ListView.view.currentItem == root) ? 'orange' : 'white'
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        root.ListView.view.currentIndex = index;
                        updateDetails(index);
                    }
                }
            }
            Column {
                id: spinnerColumn
                anchors.right: rankBox.left
                anchors.verticalCenter: rankBox.verticalCenter
                spacing: 3
                Button {
                    id: upArrow
                    normalImage: 'art/interface/pc_creation/down_arrow.png'
                    hoverImage: 'art/interface/pc_creation/down_arrow_hovered.png'
                    disabledImage: 'art/interface/pc_creation/down_arrow_disabled.png'
                    pressedImage: 'art/interface/pc_creation/down_arrow_click.png'
                    text: ''
                    rotation: 180
                    enabled: remainingSkillPoints > 0 && model.rank < model.maximumRank
                    onClicked: {
                        increaseRank(index)
                        root.ListView.view.currentIndex = index;
                    }
                }

                Button {
                    id: downArrow
                    normalImage: 'art/interface/pc_creation/down_arrow.png'
                    hoverImage: 'art/interface/pc_creation/down_arrow_hovered.png'
                    disabledImage: 'art/interface/pc_creation/down_arrow_disabled.png'
                    pressedImage: 'art/interface/pc_creation/down_arrow_click.png'
                    text: ''
                    enabled: remainingSkillPoints < availableSkillPoints && model.rank > model.minimumRank
                    onClicked: {
                        decreaseRank(index)
                        root.ListView.view.currentIndex = index;
                    }
                }
            }
            Rectangle {
                id: rankBox
                width: 38
                height: 27
                color: '#00000000'
                border.width: 1
                border.color: '#43586e'
                anchors.right: parent.right
                anchors.rightMargin: 5
                Text {
                    anchors.centerIn: parent
                    id: skillRankLabel
                    text: (model.rank / 2)
                    font.bold: true
                    font.pointSize: 17
                    color: 'white'
                    smooth: true
                }
            }
        }
    }

    Component {
        id: sectionDelegate
        StandardText {
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: "AlignHCenter"
            text: section == 'classSkill' ? "Class Skills" : "Cross Class Skills"
        }
    }

    ListModel {
        id: skillModel
    }

    ScrollView {
        id: skillView
        model: skillModel
        delegate: skillDelegate
        width: 220
        spacing: 5
        anchors.top: parent.top
        anchors.bottom: pointsRemainingLabel.top
        anchors.bottomMargin: 5
        section.delegate: sectionDelegate
        section.property: "section"
    }

    Item {
        id: detailsGroup

        width: childrenRect.width
        height: childrenRect.height

        anchors.right: parent.right
        anchors.verticalCenter: skillView.verticalCenter

        Image {
            source: '../art/interface/pc_creation/skill_breakdown.png'
            width: 85
            height: 153
        }

        StandardText {
            x: 94
            y: 2
            text: 'Skill Rank'
        }

        StandardText {
            x: 94
            y: 44
            text: 'Ability Modifier'
        }

        StandardText {
            x: 94
            y: 84
            text: 'Misc. Modifier'
        }

        StandardText {
            x: 94
            y: 128
            text: 'Total Modifier'
        }

        Text {
            id: abilityTypeLabel
            x: -1
            y: 44
            width: 40
            height: 16
            color: "#ffffff"
            text: detailsAbility
            font.bold: true
            smooth: false
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
        }

        Text {
            id: abilityModLabel
            x: 42
            y: 41
            width: 40
            height: 22
            color: "#ffffff"
            text: detailsAbilityMod
            smooth: false
            font.bold: true
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
        }

        Text {
            id: rankLabel
            x: 42
            y: 0
            width: 40
            height: 22
            color: "#ffffff"
            text: detailsRank
            smooth: false
            font.bold: true
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
        }

        Text {
            id: miscLabel
            x: 42
            y: 82
            width: 40
            height: 22
            color: "#ffffff"
            text: detailsMiscMod
            smooth: false
            font.bold: true
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
        }

        Text {
            id: totalBonusLabel
            x: 41
            y: 121
            width: 44
            height: 32
            color: "#ffffff"
            text: detailsTotalMod
            style: "Normal"
            font.pointSize: 17
            smooth: false
            font.bold: true
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
        }

    }

    StandardText {
        id: pointsRemainingLabel
        anchors.bottom: parent.bottom
        anchors.left: skillView.left
        anchors.right: skillView.right
        horizontalAlignment: "AlignHCenter"
        text: remainingSkillPoints + " skill points remaining"
    }

}
