var CombatUi = {};

(function() {

    var combatBarDialog = null;
    var initiativeBarDialog = null;
    var movementIndicatorNode = null;
    var movementIndicatorRootNode = null;

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
    function worldClicked(event, worldPosition) {
        if (!Combat.isActive())
            return;

        var participant = Combat.getActiveParticipant();

        if (participant && Party.isMember(participant) && !participant.isBusy()) {
            var path = new Path(participant, worldPosition);

            if (!path.isEmpty()) {
                var movementGoal = new MovementGoal(path, false);
                var sayHelloGoal = {
                    advance: function() {
                        gameView.scene.addTextOverlay(worldPosition, "I am there!", [0.9, 0, 0, 0.9]);
                        this.finished = true;
                    },
                    isFinished: function() {
                        return this.finished;
                    },
                    cancel: function() {
                    }
                };

                participant.setGoal(new GoalSequence(movementGoal, sayHelloGoal));
            }
        }
    }

    CombatUi.objectMouseEnter = function(object, event) {
        hideMovementIndicator();

        var participant = Combat.getActiveParticipant();

        if (participant && Party.isMember(participant) && !participant.isBusy()) {
            movementIndicatorNode = gameView.scene.createNode();
            movementIndicatorNode.position = object.position;

            var indicator = new MovementIndicator(gameView.scene, gameView.materials);
            indicator.radius = participant.radius;
            indicator.circleWidth = 3;
            movementIndicatorNode.attachObject(indicator);

            var path = Maps.currentMap.findPathIntoRange(participant, object, 25);

            if (path.length == 0) {
                indicator.fillColor = [1, 0, 0, 0.5];
                return;
            }

            movementIndicatorNode.position = path[path.length - 1];

            var length = pathLength(path);

            movementIndicatorRootNode = gameView.scene.createNode();

            for (var i = 0; i < path.length; ++i) {
                if (i + 1 < path.length) {
                    var line = new DecoratedLineRenderable(gameView.scene, gameView.materials);
                    if (length > 250)
                        line.color = [1, 0, 0, 1];
                    else
                        line.color = [0, 1, 0, 1];
                    line.addLine(path[i], path[i + 1]);
                    movementIndicatorRootNode.attachObject(line);
                }
            }
        }
    };

    CombatUi.objectMouseLeave = function(object, event) {
        return false;
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

            var participant = Combat.getActiveParticipant();

            if (Party.isMember(participant) && !participant.isBusy()) {

                var action = object.getDefaultAction(participant);

                if (!action)
                    return;

                if (!action.combat) {
                    // TODO: Notify the user
                    print("Cannot perform this action during combat.");
                    return;
                }

                var path = new Path(participant, object);

                if (!path.isEmpty()) {
                    var movementGoal = new MovementGoal(path, false);
                    var secondaryGoal = new ActionGoal(action);

                    participant.setGoal(new GoalSequence(movementGoal, secondaryGoal));
                }

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
        hideMovementIndicator();

        updateInitiative();
        updateCombatBar();

        if (previousParticipant)
            previousParticipant.setSelected(false);

        var activeParticipant = Combat.getActiveParticipant();
        activeParticipant.setSelected(true); // Show on the battlefield who is acting
    }

    function mouseMoved(event, worldPos) {
        hideMovementIndicator();

        var participant = Combat.getActiveParticipant();

        if (participant && Party.isMember(participant) && !participant.isBusy()) {
            movementIndicatorNode = gameView.scene.createNode();
            movementIndicatorNode.position = worldPos;

            var indicator = new MovementIndicator(gameView.scene, gameView.materials);
            indicator.radius = participant.radius;
            indicator.circleWidth = 3;
            movementIndicatorNode.attachObject(indicator);

            var path = Maps.currentMap.findPath(participant, worldPos);

            if (path.length == 0) {
                indicator.fillColor = [1, 0, 0, 0.5];
                return;
            }

            var length = pathLength(path);

            movementIndicatorRootNode = gameView.scene.createNode();

            for (var i = 0; i < path.length; ++i) {
                if (i + 1 < path.length) {
                    var line = new DecoratedLineRenderable(gameView.scene, gameView.materials);
                    if (length > 250)
                        line.color = [1, 0, 0, 1];
                    else
                        line.color = [0, 1, 0, 1];
                    line.addLine(path[i], path[i + 1]);
                    movementIndicatorRootNode.attachObject(line);
                }
            }
        }
    }

    function hideMovementIndicator() {
        if (movementIndicatorNode) {
            gameView.scene.removeNode(movementIndicatorNode);
            movementIndicatorNode = null;
        }
        if (movementIndicatorRootNode) {
            gameView.scene.removeNode(movementIndicatorRootNode);
            movementIndicatorRootNode = null;
        }
    }

    function goalCompleted(critter, goal) {

        if (!Combat.isActive() || !combatBarDialog)
            return;

        var participant = Combat.getActiveParticipant();

        if (participant && Party.isMember(participant) && critter === participant) {
            combatBarDialog.enabled = true;
        }
    }

    function goalStarted(critter, goal) {

        if (!Combat.isActive() || !combatBarDialog)
            return;

        var participant = Combat.getActiveParticipant();

        if (participant && Party.isMember(participant) && critter === participant) {
            combatBarDialog.enabled = false;
            hideMovementIndicator();
        }

    }

    function initialize() {
        EventBus.addListener(EventTypes.GoalStarted, goalStarted);
        EventBus.addListener(EventTypes.GoalFinished, goalCompleted);

        Combat.addCombatStartListener(CombatUi.show);
        Combat.addCombatStartListener(updateInitiative);
        Combat.addCombatEndListener(CombatUi.hide);
        Combat.addActiveParticipantChangedListener(participantChanged);

        Maps.addMouseClickListener(worldClicked);
        Maps.addMouseMoveListener(mouseMoved);
        Maps.addMouseLeaveListener(hideMovementIndicator);
    }

    StartupListeners.add(initialize, 'combat-ui', ['combat']);

})();
