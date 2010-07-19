var PartyUi = {};

(function() {

    var ui = null;

    function performAction(action, index) {
        var target;

        if (index >= 10000) {
            target = Party.getFollowers()[index - 10000];
        } else {
            target = Party.getPlayers()[index];
        }

        if (!target) {
            print("Performing action " + action + " on undefined party member: " + index);
            return;
        }

        switch (action) {
            case "select":
                if (currentSelection && currentSelection != target)
                    currentSelection.setSelected(false);
                currentSelection = target;
                target.setSelected(true);
                PartyUi.update();
                break;

            case "select_and_center":
                if (currentSelection && currentSelection != target)
                    currentSelection.setSelected(false);
                currentSelection = target;
                target.setSelected(true);
                gameView.centerOnWorld(target.position);
                PartyUi.update();
                break;

            case "charsheet":
                var renderState = target.getRenderState();
                if (renderState)
                    showMobileInfo(target, renderState.modelInstance);
        }
    }

    PartyUi.show = function() {
        if (ui)
            return;

        ui = gameView.addGuiItem('interface/PartyInterface.qml');
        ui.action.connect(performAction);

        PartyUi.update();
    };

    PartyUi.hide = function() {
        if (!ui)
            return;

        ui.deleteLater();
        ui = null;
    };

    function getModel(critter) {
        return {
            portrait: getPortrait(critter.portrait, Portrait_Medium),
            selected: currentSelection == critter
        };
    }

    /**
     * Refresh the party ui after the party ui state has changed.
     */
    PartyUi.update = function() {
        if (!ui)
            return;

        var model = [];
        Party.getPlayers().forEach(function(player) {
            model.push(getModel(player));
        });

        ui.playerCharacters = model;

        model = [];
        Party.getFollowers().forEach(function(npc) {
            model.push(getModel(npc));
        });

        ui.nonPlayerCharacters = model;
    };

    // TODO: Replace this with a global gameStarted event
    StartupListeners.add(function() {
        SaveGames.addLoadedListener(function() {
            print("Showing party ui");
            PartyUi.show();
            PartyUi.update();
        });
    });

})();
