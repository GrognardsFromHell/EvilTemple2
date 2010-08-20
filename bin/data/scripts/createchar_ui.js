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
    var currentCharacter = {

    };

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
            // TODO: set feats
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
        if (currentGender == Male)
            prototypeId = race.prototypeMale;
        else if (currentGender == Female)
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
    
    function featsChosen(feats) {

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
        currentDialog.featsChosen.connect(featsChosen);

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
