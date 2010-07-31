/**
 * This will control an active battle. Since there can only be one active battle
 * at once, a singleton will suffice here.
 */
var Combat = {
};

(function() {

    var active = false; // Is combat active right now?

    /**
     * Returns whether a battle is running at the moment.
     */
    Combat.isActive = function() {
        return active;
    }

})();
