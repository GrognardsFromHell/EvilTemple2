var CombatUi = {};

(function() {

    var combatBarDialog = null;
    var initiativeBarDialog = null;

    /**
     * Make the combat UI visible.
     */
    CombatUi.show = function() {
        if (!combatBarDialog) {
            combatBarDialog = gameView.addGuiItem('interface/CombatBar.qml');

            combatBarDialog.endTurn.connect(Combat.endTurn);
        }
        if (!initiativeBarDialog) {
            initiativeBarDialog = gameView.addGuiItem('interface/InitiativeBar.qml');
        }
    };

    CombatUi.hide = function() {
        if (combatBarDialog) {
            combatBarDialog.deleteLater();
            combatBarDialog = null;
        }
        if (initiativeBarDialog) {
            initiativeBarDialog.deleteLater();
            initiativeBarDialog = null;
        }
    };

    /**
     * This function is called whenever the world is clicked while combat is active.
     *
     * @param event The mouse event that triggered the function call.
     * @param worldPosition The position in the world that was clicked.
     */
    CombatUi.worldClicked = function(event, worldPosition) {
        print("World click @ " + worldPosition);

        var participant = Combat.getActiveParticipant();

        if (!participant || !Party.isMember(participant))
            return;

        var sceneNode = gameView.scene.createNode();
        sceneNode.position = worldPosition;

        var indicator = new MovementIndicator(gameView.scene, gameView.materials);
        indicator.radius = participant.radius;
        indicator.circleWidth = 3;
        sceneNode.attachObject(indicator);

        gameView.addVisualTimer(5000, function() {
            print("Removing scene node.");
            gameView.scene.removeNode(sceneNode);
        });
    };

    /**
     * Notifies the combat UI of an object being clicked.
     *
     * @param object The object that was clicked.
     * @param event The corresponding mouse event.
     */
    CombatUi.objectClicked = function(object, event) {

        if (event.button == Mouse.RightButton) {
            var renderState = object.getRenderState();
            if (renderState)
                showMobileInfo(object, renderState.modelInstance);
        } else if (event.button == Mouse.LeftButton) {

            var activeParticipant = Combat.getActiveParticipant();

            if (Party.isMember(activeParticipant)) {
                object.dealDamage(2, activeParticipant);
            }
        }

    };

    /**
     * Notifies the combat UI of an object being double-clicked.
     *
     * @param object The object that was clicked.
     * @param event The corresponding mouse event.
     */
    CombatUi.objectDoubleClicked = function(object, event) {
        print("Object double clicked");
    };

    function updateInitiative() {
        if (!initiativeBarDialog)
            return;

        var participants = Combat.getParticipants();
        var activeId = Combat.getActiveParticipantId();

        var initiative = participants.map(function (obj) {
            return {
                id: obj.mobile.id,
                name: obj.mobile.getName(),
                initiative: obj.initiative,
                portrait: Portraits.getImage(obj.mobile.portrait, Portrait.Small),
                effectiveDex: obj.mobile.getEffectiveDexterity(),
                active: obj.mobile.id == activeId
            };
        });

        // Sort by initiative first, then by dex modifier
        initiative.sort(function (a, b) {
            if (a.initiative != b.initiative) {
                return b.initiative - a.initiative;
            } else {
                return b.effectiveDex - a.effectiveDex
            }
        });

        initiativeBarDialog.initiative = initiative;
    }

    function updateCombatBar() {
        // If the active participant is not user-controlled, grey out the bar and disable the button
        var activeParticipant = Combat.getActiveParticipant();

        if (!Party.isMember(activeParticipant)) {
            combatBarDialog.state = 'gray';
            combatBarDialog.fillPercentage = 1;
        } else {
            combatBarDialog.state = '';
        }
    }

    function participantChanged(previousParticipant) {
        updateInitiative();
        updateCombatBar();

        if (previousParticipant)
            previousParticipant.setSelected(false);

        var activeParticipant = Combat.getActiveParticipant();
        activeParticipant.setSelected(true); // Show on the battlefield who is acting       
    }

    function hideUi() {

    }

    function initialize() {
        Combat.addCombatStartListener(CombatUi.show);
        Combat.addCombatStartListener(updateInitiative);
        Combat.addCombatEndListener(CombatUi.hide);
        Combat.addActiveParticipantChangedListener(participantChanged);
    }

    StartupListeners.add(initialize, 'combat-ui', ['combat']);

})();
