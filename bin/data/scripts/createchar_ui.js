/*
 This file controls the character creation UI.
 */

var CreateCharacterUi = {};

(function() {

    /**
     * Stages as used by the character creation UI.
     */
    var Stage = {
        Stats: 0,
        Race: 1,
        Gender: 2,
        Height: 3,
        Hair: 4,
        Class: 5,
        Alignment: 6,
        Deity: 7,
        Features: 8,
        Feats: 9,
        Skills: 10,
        Spells: 11,
        Portrait: 12,
        VoiceAndName: 13,
        Finished: 14
    };

    var successCallback = null;
    var cancelCallback = null;
    var currentDialog = null;

    /**
     * The character we're building will be recorded in this structure.
     */
    var currentRace = null; // Normally defined in the prototype
    var currentGender = null; // Normally defined in the prototype
    var currentClass = null; // Currently selected class obj
    var currentFeats = []; // Chosen feats
    var currentSkillDistribution = {}; // Skill points spent
    var currentCharacter = {
        domains: [],
        feats: [],
        skills: {},
        classLevels: []
    };

    /**
     * Indicates that a given feat can be chosen again by the user, even if it is unique.
     * For simple feats, this means the user has not yet chosen the feat, for feats with
     * an argument, it is checked that at least one option has not yet been chosen.
     *
     * @param feat The feat object to check against.
     */
    function canFeatBeTakenAgain(feat) {

        if (feat.argument) {
            // This checks that at least one of the options for this feat was not yet taken.
            return feat.argument.values.some(function (option) {
                return !currentCharacter.hasFeat([feat.id, option.id]);
            });
        } else {
            // This is the simple case, the feat itself is either in the list of selected feats or not.
            return !currentCharacter.hasFeat(feat.id);
        }

    }

    function getNumberOfAvailableBonusFeats() {
        var classLevel = currentCharacter.getClassLevels(currentClass.id);
        var bonusFeats = currentClass.getBonusFeatsAtLevel(classLevel);
        return bonusFeats ? bonusFeats.count : 0;
    }

    function getNumberOfAvailableFeats() {
        var race = Races.getById(currentCharacter.race);
        return race.startingFeats;
    }

    /**
     * Checks if the feat is a bonus-feat valid for the current selection.
     */
    function isBonusFeat(featId) {
        var classLevel = currentCharacter.getClassLevels(currentClass.id);
        var bonusFeats = currentClass.getBonusFeatsAtLevel(classLevel);
        if (!bonusFeats)
            return false;
        else
            return bonusFeats.feats.indexOf(featId) != -1;
    }

    function getNumberOfRemainingBonusFeats() {
        var remaining = getNumberOfAvailableBonusFeats();

        currentFeats.forEach(function (featInstance) {
            if (featInstance instanceof Array) {
                if (isBonusFeat(featInstance[0]))
                    remaining--;
            } else if (isBonusFeat(featInstance))
                remaining--;
        });

        if (remaining < 0)
            remaining = 0;

        return remaining;
    }

    function getNumberOfRemainingFeats() {
        var remaining = getNumberOfAvailableFeats();
        var remainingBonus = getNumberOfAvailableBonusFeats();

        currentFeats.forEach(function (featInstance) {
            var featId = (featInstance instanceof Array) ? featInstance[0] : featInstance;

            // Bonus feats are deducted from the available bonus feats first.
            if (remainingBonus > 0 && isBonusFeat(featId))
                remainingBonus--;
            else
                remaining--;
        });

        return remaining;
    }

    /**
     * Returns true if all feats have been taken.
     */
    function allFeatsTaken() {
        return getNumberOfRemainingBonusFeats() == 0
                && getNumberOfRemainingFeats() == 0;
    }

    function updateFeatsDialog() {
        var feats = Feats.getAll();
        var featsDialog = currentDialog.getFeatsDialog();

        var remainingBonusFeats = getNumberOfRemainingBonusFeats();
        var remainingFeats = getNumberOfRemainingFeats();

        featsDialog.remainingFeats = remainingFeats;
        featsDialog.remainingBonusFeats = remainingBonusFeats;

        /*
         Build the list of available feats.
         */
        var availableFeats = feats.filter(function (feat) {
            return !feat.unique || canFeatBeTakenAgain(feat);
        });

        featsDialog.availableFeats = availableFeats.map(function (feat) {
            var bonusFeat = isBonusFeat(feat.id);
            var disabled = remainingFeats == 0 && !(bonusFeat && remainingBonusFeats > 0);

            // This disables the extra hints about bonus-feats when no bonus feats remain to be selected
            if (remainingBonusFeats == 0)
                bonusFeat = false;

            var requires;
            if (feat.requirements) {
                requires = '';

                feat.requirements.forEach(function (requirement) {
                    // Skip the requirement if it's conditional.
                    if (requirement instanceof ConditionalRequirement)
                        return;

                    if (requires != '')
                        requires += ', ';

                    var requirementMet;

                    if (!feat.argument) {
                        requirementMet = requirement.isMet(currentCharacter, null)
                    } else {
                        // If it is met for a single combination, the feat should still be shown
                        requirementMet = feat.argument.values.some(function (option) {
                            return requirement.isMet(currentCharacter, option.id);
                        });
                    }

                    if (requirementMet) {
                        requires += requirement.toString();
                    } else {
                        disabled = true;
                        requires += '<font color="#cc0000">' + requirement.toString() + '</font>'
                    }
                });
            }

            return {
                id: feat.id,
                name: feat.name,
                requires: requires,
                disabled: disabled,
                bonusFeat: bonusFeat
            };
        });

        var selectedFeats = [];
        currentFeats.forEach(function (instance) {
            if (typeof(instance) == 'string') {
                selectedFeats.push(Feats.getById(instance));
            } else {
                var feat = Feats.getById(instance[0]);
                selectedFeats.push({
                    id: feat.id,
                    name: feat.getName(instance)
                });
            }
        });

        featsDialog.selectedFeats = selectedFeats;
    }

    function showFeatHelp(featId) {
        print("Showing help for feat " + featId);
    }

    /**
     * Gets the arguments of a feat that are still available.
     * @param feat The feat object.
     */
    function getAvailableFeatOptions(feat) {

        // Filter out all the options that were already taken by the character.
        var availableOptions = feat.argument.values.filter(function (option) {
            return !currentCharacter.hasFeat([feat.id, option.id]);
        });

        // Now disable those options that have requirements.
        return availableOptions.map(function (option) {
            var requires = '';

            // Go up to the feat and check requirements that are *specific* to this option
            var allRequirementsMet = feat.requirements.every(function (requirement) {
                if (requirement instanceof ConditionalRequirement) {
                    if (requirement.condition == option) {
                        // Applies to this option only
                        if (requires != '')
                            requires += ', ';
                        requires += requirement.requirement.toString();

                        if (!requirement.requirement.isMet(currentCharacter, option)) {
                            return false;
                        }
                    }
                }

                return true;
            });

            return {
                id: option.id,
                text: option.text,
                requires: requires,
                disabled: !allRequirementsMet
            };
        });
    }

    function selectFeat(featId) {
        print("Selecting feat " + featId);

        var feat = Feats.getById(featId);

        if (feat.argument) {

            var dialog = gameView.addGuiItem('interface/CreateCharacterFeatArgument.qml');
            dialog.headline = feat.argument.description;
            dialog.availableOptions = getAvailableFeatOptions(feat);
            dialog.accepted.connect(function (optionId) {
                dialog.deleteLater();

                var featInstance = [featId, optionId];

                // Finish the argument selection
                currentFeats.push(featInstance);
                currentCharacter.feats.push(featInstance);
                updateFeatsDialog();
            });
            dialog.cancelled.connect(function() {
                dialog.deleteLater();
            });
        } else {
            currentFeats.push(featId);
            currentCharacter.feats.push(featId);
            updateFeatsDialog();
        }

        if (allFeatsTaken()) {
            currentDialog.overallStage = Stage.Skills;
        }
    }

    function unselectFeat(index) {
        print("Deselecting feat " + index);
        currentCharacter.removeFeat(currentFeats[index]);
        currentFeats.splice(index, 1);
        updateFeatsDialog();

        if (!allFeatsTaken()) {
            currentDialog.overallStage = Stage.Feats;
        }
    }

    /**
     * Handles requests by the user to change the active stage.
     */
    function activeStageRequested(stage) {

        var race;

        if (stage == Stage.Race) {
            currentDialog.races = Races.getAll();

        } else if (stage == Stage.Height) {
            race = Races.getById(currentRace);

            if (currentCharacter.gender == Male) {
                currentDialog.minHeight = race.heightMale[0];
                currentDialog.maxHeight = race.heightMale[1];
            } else {
                currentDialog.minHeight = race.heightFemale[0];
                currentDialog.maxHeight = race.heightFemale[1];
            }

        } else if (stage == Stage.Class) {
            currentDialog.classes = Classes.getAll();

        } else if (stage == Stage.Alignment) {
            // Get allowable alignments (from the party alignment and the class)
            var alignments = CompatibleAlignments[Party.alignment].slice(0);

            // Check each against the selected class
            currentClass.requirements.forEach(function (requirement) {
                if (requirement.type == 'alignment') {
                    for (var i = 0; i < alignments.length; ++i) {
                        if (requirement.inclusive && requirement.inclusive.indexOf(alignments[i]) == -1
                                || requirement.exclusive && requirement.exclusive.indexOf(alignments[i]) != -1) {
                            alignments.splice(i, 1);
                            --i;
                        }
                    }
                }
            });

            currentDialog.availableAlignments = alignments;

        } else if (stage == Stage.Deity) {
            currentDialog.availableDeities = Deities.getAll().filter(function (deity) {
                return CompatibleAlignments[deity.alignment].indexOf(currentCharacter.alignment) != -1;
            });

        } else if (stage == Stage.Features) {

            if (currentClass.id == StandardClasses.Cleric) {
                var features = currentDialog.loadFeaturesPage('CreateCharacterDomains.qml');
                var deity = Deities.getById(currentCharacter.deity);
                features.availableDomains = deity.domains.map(Domains.getById);
                features.domainsSelected.connect(function(domains) {
                    if (domains.length == 2) {
                        currentCharacter.domains = domains;
                        currentDialog.overallStage = Stage.Feats;
                    } else {
                        currentDialog.overallStage = Stage.Features;
                    }
                });

            } else if (currentClass.id == StandardClasses.Ranger) {
                // TODO: Ranger favored enemy
                currentDialog.overallStage = Stage.Feats;
            } else if (currentClass.id == StandardClasses.Wizard) {
                // TODO: Wizard specialisation
                currentDialog.overallStage = Stage.Feats;
            } else {
                currentDialog.overallStage = Stage.Feats;
            }

        } else if (stage == Stage.Feats) {
            updateFeatsDialog();
        } else if (stage == Stage.Skills) {
            var skillsDialog = currentDialog.getSkillsDialog();
            var classSkills = currentClass.getClassSkills(currentCharacter);
            var ecl = currentCharacter.getEffectiveCharacterLevel();

            var maxSkillPoints = currentClass.skillPoints + getAbilityModifier(currentCharacter.intelligence);
            if (maxSkillPoints < 1)
                maxSkillPoints = 1;
            if (currentRace == StandardRaces.Human)
                maxSkillPoints += 1;
            if (ecl == 1)
                maxSkillPoints *= 4;

            var remainingSkillPoints = maxSkillPoints;

            for (var k in currentSkillDistribution) {
                if (classSkills.indexOf(k) != -1) {
                    remainingSkillPoints -= Math.floor(currentSkillDistribution[k] / 2);
                } else {
                    remainingSkillPoints -= currentSkillDistribution[k];
                }
            }

            skillsDialog.availableSkillPoints = maxSkillPoints;
            skillsDialog.remainingSkillPoints = remainingSkillPoints;
            skillsDialog.skills = Skills.getAll().map(function (skill) {
                var classSkill = classSkills.indexOf(skill.id) != -1;
                var rank = currentCharacter.skills[skill.id] ? currentCharacter.skills[skill.id] : 0;
                var currentRank = rank;
                if (currentSkillDistribution[skill.id])
                    currentRank += currentSkillDistribution[skill.id];

                return {
                    id: skill.id,
                    name: skill.name,
                    rank: currentRank,
                    classSkill: classSkill,
                    minimumRank: rank,
                    maximumRank: classSkill ? (ecl + 3) * 2 : (ecl + 3)
                };
            });
        }

        // TODO: Validate/reset stages
        currentDialog.activeStage = stage;
    }

    function updateCharacterSheet() {
        var sheet = {
            strength: currentCharacter.strength,
            dexterity: currentCharacter.dexterity,
            constitution: currentCharacter.constitution,
            intelligence: currentCharacter.intelligence,
            wisdom: currentCharacter.wisdom,
            charisma: currentCharacter.charisma,

            height: currentCharacter.height,
            weight: currentCharacter.weight
        };

        if (currentCharacter.prototype) {
            var level = currentCharacter.getEffectiveCharacterLevel();

            if (level > 0) {
                sheet.level = level;
                sheet.experience = currentCharacter.experiencePoints;
                sheet.fortitudeSave = currentCharacter.getFortitudeSave();
                sheet.willSave = currentCharacter.getWillSave();
                sheet.reflexSave = currentCharacter.getReflexSave();
                sheet.hitPoints = currentCharacter.hitPoints;

                var bab = currentCharacter.getBaseAttackBonus();
                sheet.meleeBonus = bab + getAbilityModifier(currentCharacter.getEffectiveStrength());
                sheet.rangedBonus = bab + getAbilityModifier(currentCharacter.getEffectiveDexterity());

                sheet.initiative = currentCharacter.getInitiativeBonus();
                sheet.speed = currentCharacter.getEffectiveLandSpeed();
            }
        }

        currentDialog.characterSheet = sheet;
    }

    function updateModelViewer() {
        if (!currentRace || !currentGender)
            return;

        var race = Races.getById(currentRace);

        if (!race) {
            print("Current character uses unknown race: " + currentCharacter.race);
            return;
        }

        var prototypeId;
        if (currentGender == Gender.Male)
            prototypeId = race.prototypeMale;
        else if (currentGender == Gender.Female)
            prototypeId = race.prototypeFemale;
        else
            throw "Unknown gender for current character: " + currentGender;

        print("Setting prototype of new character to " + prototypeId);

        currentCharacter.prototype = prototypeId;
        connectToPrototype(currentCharacter);

        currentDialog.getModelViewer().modelRotation = -120;

        var materials = currentDialog.getModelViewer().materials;
        var model = currentDialog.getModelViewer().modelInstance;
        model.model = models.load(currentCharacter.model);

        model.clearOverrideMaterials();

        var renderProperties = Equipment.getRenderEquipment(currentCharacter);

        // Set override materials
        for (var placeholder in renderProperties.materials) {
            var filename = renderProperties.materials[placeholder];
            model.overrideMaterial(placeholder, materials.load(filename));
        }
    }

    function statsDistributed(str, dex, con, intl, wis, cha) {
        currentCharacter.strength = str;
        currentCharacter.dexterity = dex;
        currentCharacter.constitution = con;
        currentCharacter.intelligence = intl;
        currentCharacter.wisdom = wis;
        currentCharacter.charisma = cha;

        // Every stat must be > 0 for the entire distribution to be valid.
        var valid = str > 0 && dex > 0 && con > 0 && intl > 0 && wis > 0 && cha > 0;

        if (valid) {
            currentDialog.overallStage = Stage.Race;
        } else {
            // This resets the overall stage back to the current one if the stats are being
            // invalidated.
            currentDialog.overallStage = Stage.Stats;
        }

        updateCharacterSheet();
    }

    function raceChosen(race) {
        currentRace = race;
        print("Race selected: " + race);
        currentDialog.overallStage = Stage.Gender;
        updateModelViewer();
    }

    function genderChosen(gender) {
        currentGender = gender;
        print("Gender selected: " + gender);
        currentDialog.overallStage = Stage.Height;
        updateModelViewer();
    }

    function heightChosen(height) {
        var race = Races.getById(currentRace);
        var raceWeight = (currentGender == Male) ? race.weightMale : race.weightFemale;
        var raceHeight = (currentGender == Male) ? race.heightMale : race.heightFemale;

        var heightInCm = Math.floor(raceHeight[0] + (raceHeight[1] - raceHeight[0]) * height);
        var weightInLb = Math.floor(raceWeight[0] + (raceWeight[1] - raceWeight[0]) * height);

        currentCharacter.height = heightInCm; // This will also affect rendering-scale.
        currentCharacter.weight = weightInLb;

        // TODO: This formula is most likely wrong.
        /*
         Attempt at fixing this:
         Assume that the 0cm is scale 0 and the medium height between min/max (0.5f) is scale 1
         So for a height-range of 100cm-200cm, with a default of 150, the scale-range would be
         0.66 + (1.33 - 0.66) * heightFactor, where 0.66 = 100/150 and 1.33 = 200/150
         */
        var midHeight = raceHeight[0] + (raceHeight[1] - raceHeight[0]) * 0.5;
        var minFac = raceHeight[0] / midHeight;
        var maxFac = raceHeight[1] / midHeight;

        var adjustedHeightFac = minFac + (maxFac - minFac) * height;

        currentDialog.getModelViewer().modelScale = currentCharacter.scale / 100 * adjustedHeightFac;

        // Height changing never changes the state unless it is to advance it
        if (currentDialog.overallStage < Stage.Hair)
            currentDialog.overallStage = Stage.Class;

        updateCharacterSheet();
    }

    function classChosen(classId) {
        var classObj = Classes.getById(classId);
        print("Chose class: " + classObj.name);

        print("Hit Die: " + classObj.getHitDie(1).getMaximum());

        // Give the character a corresponding class level
        currentCharacter.classLevels = [];
        classObj.addClassLevel(currentCharacter);

        currentClass = classObj;

        updateCharacterSheet();

        if (currentDialog.overallStage < Stage.Alignment)
            currentDialog.overallStage = Stage.Alignment;
    }

    function alignmentChosen(alignment) {
        currentCharacter.alignment = alignment;
        currentDialog.overallStage = Stage.Deity;
    }

    function deityChosen(deity) {
        currentCharacter.deity = deity;
        currentDialog.overallStage = Stage.Features;
    }

    function requestSkillDetails(skillId, skills) {
        var skillsDialog = currentDialog.getSkillsDialog();

        var skill = Skills.getById(skillId);
        var rank = Math.floor(skills[skillId] / 2);
        skillsDialog.detailsRank = rank;
        var abilityValue;
        switch (skill.ability) {
            case Abilities.Strength:
                skillsDialog.detailsAbility = 'STR';
                abilityValue = currentCharacter.getEffectiveStrength();
                break;
            case Abilities.Dexterity:
                skillsDialog.detailsAbility = 'DEX';
                abilityValue = currentCharacter.getEffectiveDexterity();
                break;
            case Abilities.Constitution:
                skillsDialog.detailsAbility = 'CON';
                abilityValue = currentCharacter.getEffectiveConstitution();
                break;
            case Abilities.Intelligence:
                skillsDialog.detailsAbility = 'INT';
                abilityValue = currentCharacter.getEffectiveIntelligence();
                break;
            case Abilities.Wisdom:
                skillsDialog.detailsAbility = 'WIS';
                abilityValue = currentCharacter.getEffectiveWisdom();
                break;
            case Abilities.Charisma:
                skillsDialog.detailsAbility = 'CHA';
                abilityValue = currentCharacter.getEffectiveCharisma();
                break;
        }

        abilityValue = getAbilityModifier(abilityValue);
        skillsDialog.detailsAbilityMod = abilityValue;

        var skillRanks = {};
        for (var k in skills) {
            skillRanks[k] = Math.floor(skills[k] / 2);
        }
        var miscMod = skill.getSynergyBonus(skillRanks); // TODO: Synergy and other bonuses

        skillsDialog.detailsMiscMod = miscMod;
        skillsDialog.detailsTotalMod = rank + miscMod + abilityValue;
    }

    function skillPointsDistributed() {
        var skillsDialog = currentDialog.getSkillsDialog();
        if (skillsDialog.remainingSkillPoints == 0) {
            currentSkillDistribution = skillsDialog.skillPointDistribution; // Finalize only when finished

            // TODO: Go to spells selection if class has selectable spells (memorization isn't done here)
            currentDialog.overallStage = Stage.Portrait;
        }
    }

    CreateCharacterUi.show = function(_successCallback, _cancelCallback) {
        if (currentDialog) {
            CreateCharacterUi.cancel();
        }

        successCallback = _successCallback;
        cancelCallback = _cancelCallback;

        // Open the first stage of the dialog
        currentDialog = gameView.addGuiItem('interface/CreateCharacter.qml');

        // Connect all necessary signals
        currentDialog.activeStageRequested.connect(activeStageRequested);
        currentDialog.statsDistributed.connect(statsDistributed);
        currentDialog.raceChosen.connect(raceChosen);
        currentDialog.genderChosen.connect(genderChosen);
        currentDialog.heightChosen.connect(heightChosen);
        currentDialog.classChosen.connect(classChosen);
        currentDialog.alignmentChosen.connect(alignmentChosen);
        currentDialog.deityChosen.connect(deityChosen);

        // Connect to the signals of the feats sub-dialog
        var featDialog = currentDialog.getFeatsDialog();
        featDialog.helpRequested.connect(showFeatHelp);
        featDialog.featAddRequested.connect(selectFeat);
        featDialog.featRemoveRequested.connect(unselectFeat);

        var skillsDialog = currentDialog.getSkillsDialog();
        skillsDialog.requestDetails.connect(requestSkillDetails);
        skillsDialog.skillPointDistributionChanged.connect(skillPointsDistributed);

        // Start with the stats page
        currentDialog.overallStage = Stage.Stats;
        currentDialog.activeStage = Stage.Stats;
    };

    CreateCharacterUi.cancel = function() {
        if (!currentDialog)
            return;

        currentDialog.deleteLater();
        currentDialog = null;

        if (cancelCallback)
            cancelCallback();

        cancelCallback = null;
        successCallback = null;
    };

})();
