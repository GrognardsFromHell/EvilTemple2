
var JournalUi = {};

(function() {

    var journalDialog = null;

    JournalUi.show = function() {
        if (journalDialog)
            return;

        journalDialog = gameView.addGuiItem("interface/Journal.qml");
        journalDialog.closeClicked.connect(JournalUi.close);
        journalDialog.quests = Quests.getKnown();
    };

    JournalUi.close = function() {
        if (!journalDialog)
            return;

        journalDialog.deleteLater();
        journalDialog = null;
    };

})();
