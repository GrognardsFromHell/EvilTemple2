/**
 * This will control an active battle. Since there can only be one active battle
 * at once, a singleton will suffice here.
 */
var Combat = {
};

(function() {

    var combatStartListeners = new ListenerQueue;

    var combatEndListeners = new ListenerQueue;

    var active = false; // Is combat active right now?

    var participants = {
    };

    var activeParticipant = ''; // Gives the *id* of the active participant

    var activeParticipantChangedListener = new ListenerQueue;

    /**
     * Gets a descending list of participant ids, sorted by initiative.
     */
    function getDescendingInitiativeList() {
        var result = [];
        for (var k in participants) {
            result.push(k);
        }

        result.sort(function(a, b) {
            var participantA = participants[a];
            var participantB = participants[b];

            if (participantA.initiative != participantB.initiative) {
                return participantB.initiative - participantA.initiative;
            } else {
                return participantB.mobile.getEffectiveDexterity() - participantA.mobile.getEffectiveDexterity();
            }
        });

        return result;
    }

    /**
     * Select the next combat participant and give him a chance to act.
     */
    function proceedWithNextParticipant() {
        var initiative = getDescendingInitiativeList();

        // Look for the *current* id
        var currentIdx = initiative.indexOf(activeParticipant);

        var participant = null;
        if (currentIdx == -1 || currentIdx + 1 >= initiative.length) {
            print("Start of first round or end of round reached, continuing @ participant 0");
            participant = participants[initiative[0]];
        } else {
            participant = participants[initiative[currentIdx + 1]];
        }

        var previous;
        if (activeParticipant)
            previous = participants[activeParticipant].mobile;
        activeParticipant = participant.mobile.id;
        activeParticipantChangedListener.notify(previous);
    }

    /**
     * Returns whether a battle is running at the moment.
     */
    Combat.isActive = function() {
        return active;
    };

    /**
     * Adds a mobile to the currently running battle.
     *
     * @param participant The mobile to add.
     */
    Combat.addParticipant = function(participant) {
        if (!active)
            throw "Cannot add participants to a battle if combat isn't running.";

        var id = participant.id;

        if (!id)
            throw "Cannot add a participant without a GUUID to the combat.";

        if (participants[id]) {
            print("Trying to add participant " + id + ", which is already in combat.");
            return;
        }

        participants[id] = {
            id: id,
            initiative: participant.rollInitiative(),
            mobile: participant
        };
    };

    /**
     * Start combat, given a triggerer and initial list of participants.
     * @param triggerer The critter who caused combat to be started.
     * @param _participants An array of additional participants in the combat.
     */
    Combat.start = function(triggerer, _participants) {
        if (active)
            return;

        active = true;

        // Play combat start sound
        combatStartListeners.notify();

        Party.getMembers().forEach(Combat.addParticipant);

        _participants.forEach(Combat.addParticipant);

        proceedWithNextParticipant();
    };

    Combat.getParticipants = function() {
        var result = [];
        for (var k in participants) {
            result.push(participants[k]);
        }
        return result;
    };

    /**
     * Returns the unique id of the combat participant who is currently acting.
     */
    Combat.getActiveParticipantId = function() {
        return activeParticipant;
    };

    /**
     * Returns the mobile that is currently taking it's turn at combat.
     */
    Combat.getActiveParticipant = function() {
        var participant = participants[activeParticipant];
        if (!participant)
            return null;
        else
            return participants[activeParticipant].mobile;
    };

    /**
     * Ends the turn of the current participant.
     */
    Combat.endTurn = function() {
        if (Combat.checkEndConditions())
            return;

        if (!activeParticipant)
            throw "Cannot end the turn, since there is no active participant!";

        proceedWithNextParticipant();
    };

    Combat.checkEndConditions = function() {

        var endCombat = Combat.getParticipants().every(function (participant) {
            return participant.mobile.getReaction() != Reaction.Hostile || participant.mobile.isUnconscious()
                    || participant.mobile.map !== Maps.currentMap;
        });

        if (endCombat) {
            participants = {};
            activeParticipant = '';
            active = false;
            combatEndListeners.notify();
        }

        return endCombat;

    };

    /**
     * Checks whether a single party member will cause combat to start. Use this function if this
     * party member was moved or updated.
     *
     * @param member The party member to check.
     */
    Combat.checkCombatConditionsForPlayer = function(member) {

        var combatTriggerRange = 500; // How is this determined?

        // Gather all NPCs in the vicinity
        var vicinity = Maps.currentMap.vicinity(member.position, combatTriggerRange, NonPlayerCharacter);

        var participants = vicinity.filter(function (critter) {

            // Never take disabled/dont-draw creatures into account
            if (critter.disabled || critter.dontDraw)
                return false;

            // Skip disabled NPCs (TODO: Update this when isDisabled is available)
            if (critter.isUnconscious())
                return false;

            // Skip non-hostile NPCs
            if (critter.getReaction() != Reaction.Hostile)
                return false;

            // Check for visibility
            if (!critter.canSee(member))
                return false;

            // TODO: check other conditions
            return true;
        });

        if (participants.length > 0 && !active) {
            Combat.start(null, participants);
        }

        return participants.length > 0;
    };

    /**
     * Checks whether combat should be started due to hostile NPCs coming into view.
     */
    Combat.checkCombatConditions = function() {

        if (active || !Maps.currentMap)
            return;

        Party.getMembers().forEach(Combat.checkCombatConditionsForPlayer);
    };

    Combat.addCombatStartListener = function(listener) {
        combatStartListeners.append(listener);
    };

    Combat.addCombatEndListener = function(listener) {
        combatEndListeners.append(listener);
    };

    Combat.addActiveParticipantChangedListener = function(listener) {
        activeParticipantChangedListener.append(listener);
    };

    function initialize() {
        var check = function() {
            Combat.checkCombatConditions();
            gameView.addVisualTimer(1000, check);
        };
        gameView.addVisualTimer(1000, check);
    }

    StartupListeners.add(initialize, "combat");

})();
