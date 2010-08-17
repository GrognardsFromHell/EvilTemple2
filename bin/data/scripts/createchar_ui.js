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
    var currentCharacter = {

    };

    /**
     * Handles requests by the user to change the active stage.
     */
    function activeStageRequested(stage) {

        if (stage == Stage.Race) {
            currentDialog.races = Races.getAll();
        }

        // TODO: Validate/reset stages
        currentDialog.activeStage = stage;
    }

    function updateModelViewer() {
        if (!currentCharacter.race || !currentCharacter.gender)
            return;

        var race = Races.getById(currentCharacter.race);

        if (!race) {
            print("Current character uses unknown race: " + currentCharacter.race);
            return;
        }

        var appearance;
        if (currentCharacter.gender == Male)
            appearance = race.maleAppearance;
        else if (currentCharacter.gender == Female)
            appearance = race.femaleAppearance;
        else
            throw "Unknown gender for current character: " + currentCharacter.gender;

        currentDialog.getModelViewer().modelRotation = -120;

        var materials = currentDialog.getModelViewer().materials;
        var model = currentDialog.getModelViewer().modelInstance;
        model.model = models.load(appearance.model);

        // Override preselected models
        model.clearOverrideMaterials();

        for (var k in appearance.materials) {
            var material = materials.load(appearance.materials[k]);
            if (material)
                model.overrideMaterial(k, material);
            else
                print("Unknown material for modelviewer: " + appearance.materials[k]);
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
    }

    function raceChosen(race) {
        currentCharacter.race = race;
        print("Race selected: " + race);
        currentDialog.overallStage = Stage.Gender;
        updateModelViewer();
    }

    function genderChosen(gender) {
        currentCharacter.gender = gender;
        print("Gender selected: " + gender);
        currentDialog.overallStage = Stage.Height;
        updateModelViewer();
    }

    function heightChosen(height) {
        currentCharacter.height = height; // This will also affect rendering-scale.

        // TODO: At this point, the character model is fully specified and may already be shown in the
        // char creation dialog.

        currentDialog.getModelViewer().modelScale = height / 150;

        // Height changing never changes the state unless it is to advance it
        if (currentDialog.overallStage < Stage.Hair)
            currentDialog.overallStage = Stage.Hair;
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
