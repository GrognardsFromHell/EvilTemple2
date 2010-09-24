var JournalUi = {};

(function() {

    var journalDialog = null;

    var questTexts = readJson('quests.js');

    var reputationsTexts = readJson('reputations.js');

    JournalUi.show = function() {
        if (journalDialog)
            return;

        journalDialog = gameView.addGuiItem("interface/Journal.qml");
        journalDialog.closeClicked.connect(JournalUi.close);

        var knownQuests = Quests.getKnown();
        var quests = [];
        for (var k in knownQuests) {
            var quest = {
                'id': k,
                'state': knownQuests[k]
            };
            var texts = questTexts[k];
            if (!texts) {
                quest.name = 'Unknown quest: ' + k;
                quest.description = '';
            } else {
                quest.name = texts.name;
                quest.description = texts.description;
            }
            quests.push(quest);
        }
        journalDialog.quests = quests;

        var reputations = [];
        Reputation.getActive().forEach(function (reputationId) {
            reputations.push({
                id: reputationId,
                name: reputationsTexts[reputationId].name,
                description: reputationsTexts[reputationId].description,
                effect: reputationsTexts[reputationId].effect
            });
        });

        journalDialog.reputations = reputations;
    };

    JournalUi.close = function() {
        if (!journalDialog)
            return;

        journalDialog.deleteLater();
        journalDialog = null;
    };

})();
