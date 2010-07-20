
/*
    This JS file allows for tests to be run outside of the normal UI.
*/
var testMode = (gameView === undefined); // Test mode is active if there is no gameView available.

function fillQuests(root) {
    if (!testMode)
        return;

    print("TestMode active");

    var quests = [];
    quests.push({
        'id': 'quest-6',
        'state': 'mentioned',
        'name': 'Unhappy Tailor',
        'description': "Jinnerth, the tailor of Hommlet, has asked you to speak with the captain of the militia about joining the town militia."
    });
    quests.push({
        'id': 'quest-16',
        'state': 'accepted',
        'name': 'Unhappy Tailor',
        'description': "Jinnerth, the tailor of Hommlet, has asked you to speak with the captain of the militia about joining the town militia."
    });
    root.quests = quests;
}
