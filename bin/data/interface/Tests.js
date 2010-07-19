
/*
    This JS file allows for tests to be run outside of the normal UI.
*/
var testMode = (gameView === undefined); // Test mode is active if there is no gameView available.

function fillQuests(root) {
    if (!testMode)
        return;

    print("TestMode active");

    var quests = {};
    quests['quest-6'] = 'completed';
    root.quests = quests;
}
