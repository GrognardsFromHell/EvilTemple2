/**
 * Quest state manager.
 */
var Quests = {};

(function() {
    // Constants for quest state
    var Mentioned = 0;
    var Accepted = 1;

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
     * Checks whether a quest is no longer unknown to the party.
     * @param id The quest id.
     */
    Quests.isKnown = function(id) {
        return questMap[id] !== undefined;
    };
})();
