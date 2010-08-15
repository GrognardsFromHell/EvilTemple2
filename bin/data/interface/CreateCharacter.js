
/*
    Companion script for the character creation screen.
*/

// Various stages of the character creation process. They're monotonically increasing and thus are
// in an ordered relation.
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

// Stage of the character creation process.
var overallStage = Stage.Stats;

function updateButtonState() {
    statsButton.done = overallStage > Stage.Stats;
    raceButton.enabled = overallStage >= Stage.Race;
    raceButton.done = overallStage > Stage.Race;
    genderButton.enabled = overallStage >= Stage.Gender;
    genderButton.done = overallStage > Stage.Gender;
    heightButton.enabled = overallStage >= Stage.Height;
    heightButton.done = overallStage > Stage.Height;
    hairButton.enabled = overallStage >= Stage.Hair;
    hairButton.done = overallStage > Stage.Hair;
    classButton.enabled = overallStage >= Stage.Class;
    classButton.done = overallStage > Stage.Class;
    alignmentButton.enabled = overallStage >= Stage.Alignment;
    deityButton.enabled = overallStage >= Stage.Deity;
    featuresButton.enabled = overallStage >= Stage.Features;
    featsButton.enabled = overallStage >= Stage.Feats;
    skillsButton.enabled = overallStage >= Stage.Skills;
    spellsButton.enabled = overallStage >= Stage.Spells;
    portraitButton.enabled = overallStage >= Stage.Portrait;
    voiceAndNameButton.enabled = overallStage >= Stage.VoiceAndName;
    finishButton.enabled = overallStage >= Stage.Finished;
}

function finishStage() {
    overallStage++;
    updateButtonState();
}

/**
    It's possible to "unfinish" a stage by returning to it and
    putting it into an invalid state.
*/
function unfinishStage(stage) {
    overallStage = stage;
    updateButtonState();
}

function setStageCompleted(stage, completed) {
    if (completed)
        finishStage(stage);
    else
        unfinishStage(stage);
}

/**
  Called when the dialog is shown.
  */
function initialize() {
    updateButtonState();
    state = 'stats-roll';
}
