/**
 * Models the utility bar, which is practically a toolbar on the bottom right
 * corner of the screen.
 */
var UtilityBarUi = {};

(function() {

    var utilityBar = null;

    function openTownmap() {
        WorldMapUi.show();
    }

    function openJournal() {
        JournalUi.show();
    }

    function openHelp() {
        HelpUi.show();
    }

    StartupListeners.add(function() {
        WorldMap.addMarkedNewAreaListener(function() {
            if (utilityBar)
                utilityBar.flashTownmap();
        });
    });

    /**
     * Shows the utility bar.
     */
    UtilityBarUi.show = function() {
        if (utilityBar)
            return;

        utilityBar = gameView.addGuiItem('interface/UtilityBar.qml');
        utilityBar.openTownmap.connect(openTownmap);
        utilityBar.openLogbook.connect(openJournal);
        utilityBar.openHelp.connect(openHelp);
        this.update();
    };

    /**
     * Hides the utility bar, if it's currently visible.
     */
    UtilityBarUi.close = function() {
        if (!utilityBar)
            return;
        utilityBar.deleteLater();
    };

    /**
     * Updates the state of the utility bar.
     */
    UtilityBarUi.update = function() {
        if (utilityBar) {
            utilityBar.restingStatus = RandomEncounter.getRestingStatus();
            utilityBar.currentTime = GameTime.get();
        }
    };

    StartupListeners.add(function() {
        // TODO: Replace this with a global gameStarted event
        SaveGames.addLoadedListener(UtilityBarUi.show, UtilityBarUi);
        GameTime.addTimeChangedListener(UtilityBarUi.update, UtilityBarUi);
    });

})();
