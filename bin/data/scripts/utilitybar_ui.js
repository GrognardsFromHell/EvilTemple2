
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
    };

    /**
     * Hides the utility bar, if it's currently visible.
     */
    UtilityBarUi.close = function() {
        if (!utilityBar)
            return;
        utilityBar.deleteLater();
    };

})();
