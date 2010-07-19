/**
 * Quest state manager.
 */
var Quests = {};

var StoryState = 0; // Progress of the current story

(function() {
    // Constants for quest states
    var Mentioned = 0;
    var Accepted = 1;
    var Completed = 2;
    var Botched = 3;

    var questMap = {};

    /**
     * Checks whether a quest is unknown to the party.
     * @param id The id of the quest.
     */
    Quests.isUnknown = function(id) {
        return questMap[id] === undefined;
    };

    /**
     * Checks whether a quest has been mentioned to the party.
     * @param id The id of the quest.
     */
    Quests.isMentioned = function(id) {
        return questMap[id] === Mentioned;
    };

    /**
     * Checks whether a quest has been accepted by the part.
     * @param id The id of the quest.
     */
    Quests.isAccepted = function(id) {
        return questMap[id] === Accepted;
    };

    /**
     * Checks whether a quest has been completed successfully.
     * @param id The id of the quest.
     */
    Quests.isCompleted = function(id) {
        return questMap[id] === Completed;
    };    

    /**
     * Checks whether a quest has been botched.
     * @param id The id of the quest.
     */
    Quests.isBotched = function(id) {
        return questMap[id] === Botched;
    };
    
    /**
     * Checks whether a quest is no longer unknown to the party.
     * @param id The quest id.
     */
    Quests.isKnown = function(id) {
        return questMap[id] !== undefined;
    };

    /**
     * Checks whether a quest has been accepted or finished.
     * @param id The id of the quest.
     */
    Quests.isStarted = function(id) {
        return questMap[id] >= Accepted;
    };

    /**
     * Checks whether a quest has been completed (or botched).
     * @param id The id of the quest.
     */
    Quests.isFinished = function(id) {
        return questMap[id] >= Completed;
    };

    /**
     * Mention a quest to the party, so it will show up in the questlog.
     * @param id Id of the quest.
     */
    Quests.mention = function(id) {
        if (!Quests.isKnown(id)) {
            print("Mentioning quest " + id + " to player.");
            questMap[id] = Mentioned;
        }
    };

    /**
     * Accepts a quest, if it's not already accepted or completed.
     * @param id Id of the quest.
     */
    Quests.accept = function(id) {
        if (!Quests.isStarted(id)) {
            print("Accepting quest " + id);
            questMap[id] = Accepted;
        }
    };

    /**
     * Completes a quest.
     * @param id The id of the quest to complete.
     */
    Quests.complete = function(id) {
        if (!Quests.isFinished(id)) {
            print("Completing quest " + id);
            questMap[id] = Completed;
        } else {
            print("Quest is already finished, why complete it?");
        }
    };

    /**
     * Get all known (and beyond) quests.
     */
    Quests.getKnown = function() {
        var result = [];

        for (var k in questMap) {
            switch (questMap[k]) {
            case Mentioned:
                result[k] = QuestState.Mentioned;
                break;
            case Accepted:
                result[k] = QuestState.Accepted;
                break;
            case Completed:
                result[k] = QuestState.Completed;
                break;
            case Botched:
                result[k] = QuestState.Botched;
                break;
            }
        }

        return result;
    };

    function save(payload) {
        payload.storyState = StoryState;
        payload.quests = payload;
    }

    function load(payload) {
        StoryState = payload.storyState;
        payload.quests = payload;
    }

    StartupListeners.add(function() {
        SaveGames.addSavingListener(save);
        SaveGames.addLoadingListener(load);
    });

})();
