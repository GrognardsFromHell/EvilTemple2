/**
 * This object controls random encounters.
 */
var RandomEncounter = {};

(function() {

    // Update this before every invocation
    var LegacyRandomEncounterPrototype = {
        game: {
            leader: {
                area: '',
                map: ''
            }
        },

        SLEEP_SAFE: RestingStatus.Safe,
        SLEEP_DANGEROUS: RestingStatus.Dangerous,
        SLEEP_IMPOSSIBLE: RestingStatus.Impossible,
        SLEEP_PASS_TIME_ONLY: RestingStatus.PassTimeOnly

    };

    var legacyScript = eval('(' + readFile('legacy/scripts/random_encounter.js') + ')');

    legacyScript.__proto__ = LegacyRandomEncounterPrototype;

    /**
     * Get the resting status for the current area.
     */
    RandomEncounter.getRestingStatus = function() {
        if (Maps.currentMap) {
            LegacyRandomEncounterPrototype.game.leader.area = Maps.currentMap.area;
            LegacyRandomEncounterPrototype.game.leader.map = Maps.currentMap.id;

            return legacyScript.can_sleep();
        } else {
            return RestingStatus.Impossible;
        }
    };

})();
