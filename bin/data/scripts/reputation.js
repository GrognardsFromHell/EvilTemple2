/**
 * Object for managing reputation the party has achieved.
 */
var Reputation = {};

(function() {

    var reputations = {};

    /**
     * Grants a reputation to the party.
     *
     * @param id The id of the reputation.
     */
    Reputation.add = function(id) {
        var reputation = reputations[id];

        if (!reputation) {
            reputations[id] = {
                status: true,
                added: GameTime.getReference()
            };
            print("Added a reputation to the party: " + id);
        } else if (!reputation.status) {
            reputation.status = true;
            reputation.added = GameTime.getReference();
            print("Added a reputation to the party: " + id);
        }
    };

    Reputation.remove = function(id) {
        var reputation = reputations[id];

        if (reputation) {
            reputation.status = false;
            reputation.removed = GameTime.getReference();
            print("Removed a reputation to the party: " + id);
        }
    };

    Reputation.getActive = function() {
        var result = [];

        for (var k in reputations) {
            if (reputations[k].status) {
                result.push(k);
            }
        }

        return result;
    };

    Reputation.has = function(id) {
        var reputation = reputations[id];

        if (reputation) {
            return reputation.status;
        }

        return false;
    };

    function load(payload) {
        reputations = payload.reputations;
        if (!reputations) {
            print("Warning: Savegame didn't contain proper reputations array");
            reputations = {};
        }
    }

    function save(payload) {
        payload.reputations = reputations;
    }

    StartupListeners.add(function() {
        SaveGames.addLoadingListener(load);
        SaveGames.addSavingListener(save);
    })

})();
