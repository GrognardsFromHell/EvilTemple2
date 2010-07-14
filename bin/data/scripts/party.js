
/**
 * Manages the party and its members.
 */
var Party = {};

(function() {

    /**
     * The overall money held by the party.
     */
    Party.money = new Money();

    /**
     * The main party alignment. This defines the opening vignette and motivational quest, and also limits the
     * choice of alignments the player characters may have.
     */
    Party.alignment = TrueNeutral;

    /**
     * The current player character members of the party.
     */
    var playerCharacters = [];

    /**
     * NPC followers currently in the party.
     */
    var followers = [];

    /**
     * Returns all players that are in the party.
     */
    Party.getPlayers = function() {
        // Copy the array
        var result = [];
        for (var i = 0; i < playerCharacters.length; ++i) {
            result[i] = playerCharacters[i];
        }
        return result;
    };

    /**
     * Returns all followers that are in the party.
     */
    Party.getFollowers = function() {
        // Copy the array
        var result = [];
        for (var i = 0; i < followers.length; ++i) {
            result[i] = followers[i];
        }
        return result;
    };

    /**
     * Returns an array with both members and followers that are in the party.
     */
    Party.getMembers = function () {
        var result = [];
        var i;
        for ( i = 0; i < playerCharacters.length; ++i)
            result.push(playerCharacters[i]);
        for (i = 0; i < followers.length; ++i)
            result.push(followers[i]);
        return result;
    };

    /**
     * Checks whether a given critter is part of the party.
     */
    Party.isMember = function(critter) {
        if (playerCharacters.indexOf(critter) != -1)
            return true;

        return followers.indexOf(critter) != -1;
    };

    /**
     * Adds a given player character to the party.
     * @param playerCharacter The player character to add to the party.
     * @returns True if the player character was added, false if he was already in the party.
     */
    Party.addMember = function(playerCharacter) {
        if (playerCharacters.indexOf(playerCharacter) != -1)
            return false;

        /*if (!(playerCharacter instanceof PlayerCharacter)) {
            throw "Trying to add a non-player character as a member to the party.";
        }*/

        print("Adding member to party: " + playerCharacter.id);
        
        playerCharacters.push(playerCharacter);

        playerCharacter.joinedParty();

        PartyUi.update();
        return true;
    };

    /**
     * Adds a follower to the party.
     * @param npc The NPC that should be added as an follower to this part.
     * @returns True if the follower was added, false if it was already a follower.
     */
    Party.addFollower = function(npc) {
        if (followers.indexOf(npc) != -1)
            return false;

        /*if (!(npc instanceof NonPlayerCharacter)) {
            throw "Trying to add a player character as a follower to the party.";
        }*/

        print("Adding member to party: " + npc.id);

        followers.push(npc);

        npc.joinedParty();

        PartyUi.update();
        return true;
    };

})();
