
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
    alignmentButton.done = overallStage > Stage.Alignment;
    deityButton.enabled = overallStage >= Stage.Deity;
    deityButton.done = overallStage > Stage.Deity;
    featuresButton.enabled = overallStage >= Stage.Features;
    featuresButton.done = overallStage > Stage.Features;
    featsButton.enabled = overallStage >= Stage.Feats;
    featsButton.done = overallStage > Stage.Feats;
    skillsButton.enabled = overallStage >= Stage.Skills;
    skillsButton.done = overallStage > Stage.Skills;
    spellsButton.enabled = overallStage >= Stage.Spells;
    spellsButton.done = overallStage > Stage.Spells;
    portraitButton.enabled = overallStage >= Stage.Portrait;
    portraitButton.done = overallStage > Stage.Portrait;
    voiceAndNameButton.enabled = overallStage >= Stage.VoiceAndName;
    voiceAndNameButton.done = overallStage > Stage.VoiceAndName;
    finishButton.enabled = overallStage >= Stage.Finished;
}

/**
    Shows the correct panel depending on the active stage.
  */
function updateActiveStage() {
    switch (activeStage) {
    case Stage.Stats:
        state = 'stats-roll';
        break;
    case Stage.Race:
        state = 'race';
        break;
    case Stage.Gender:
        state = 'gender';
        break;
    case Stage.Height:
        state = 'height';
        break;
    case Stage.Hair:
        state = 'hair';
        break;
    case Stage.Class:
        state = 'class';
        break;
    case Stage.Alignment:
        state = 'alignment';
        break;
    case Stage.Deity:
        state = 'deity';
        break;
    case Stage.Features:
        state = 'features';
        break;
    case Stage.Feats:
        state = 'feats';
        break;
    case Stage.Skills:
        state = 'skills';
        break;
    case Stage.Spells:
        state = 'spells';
        break;
    case Stage.Portrait:
        state = 'portrait';
        break;
    case Stage.VoiceAndName:
        state = 'voiceAndName';
        break;
    }
}

/**
  Called when the dialog is shown.
  */
function initialize() {
    updateButtonState();
    updateActiveStage();
}
