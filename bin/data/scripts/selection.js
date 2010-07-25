/**
 * This object manages the selected characters.
 */
var Selection = {};

(function() {

    var changeListener = new ListenerQueue;

    // This array contains all the selected mobiles.
    var currentSelection = [];

    /**
     * Adds a new change listener, which will be notified when the current selection changes.
     * The callback will be called with two arguments: added, removed. The first contains a list
     * of all the critters that were added from the selection, while the latter contains a list
     * of all the critters that were removed from the selection.
     *
     * @param callback The function to call when the selection changes.
     * @param thisObj The this object in whose context the callback should be called.
     */
    Selection.addChangeListener = function(callback, thisObj) {
        changeListener.append(callback, thisObj);
    };

    /**
     * Removes a change listener from this object.
     *
     * @param callback The callback to remove.
     * @param thisObj The this context of the callback.
     */
    Selection.removeChangeListener = function(callback, thisObj) {
        changeListener.remove(callback, thisObj);
    };

    /**
     * Adds critters to the current selection if they're not already selected.
     *
     * @param critters The critters to add. Only party members that are not already selected will be added.
     * @return True if at least one critter was selected.
     */
    Selection.add = function(critters) {
        var added = [];

        // Filter out non-selectable critters.
        critters.forEach(function(critter) {
            if (currentSelection.indexOf(critter) != -1)
                return;

            if (!editMode && !Party.isMember(critter)) {
                print("Trying to select critter " + critter.id + ", which is not part of the party.");
                return;
            }

            currentSelection.push(critter);
            added.push(critter);

            critter.setSelected(true);
        });

        if (added.length > 0) {
            changeListener.notify(added, []);
            return true;
        } else {
            return false;
        }
    };

    /**
     * Removes either a single or multiple critters from the current selection.
     *
     * @param critters A list of critters to remove from the selection.
     * @returns True if at least one critter was removed.
     */
    Selection.remove = function(critters) {
        var removed = [];

        // Filter out non-selectable critters.
        critters.forEach(function(critter) {
            var index = currentSelection.indexOf(critter);

            if (index != -1) {
                currentSelection.splice(index, 1);
                removed.push(critter);
                critter.setSelected(false);
            }
        });

        if (removed.length > 0) {
            changeListener.notify([], removed);
            return true;
        } else {
            return false;
        }
    };

    /**
     * Checks whether a critter is selected or not.
     * @param critter The critter to check.
     */
    Selection.isSelected = function(critter) {
        return currentSelection.indexOf(critter) != -1;
    };

    /**
     * Gets a copy of the current selection.
     * @returns A list of the currently selected mobiles.
     */
    Selection.get = function() {
        return currentSelection.slice(0);
    };

    /**
     * Clears the entire selection.
     */
    Selection.clear = function() {
        var removed = currentSelection;
        currentSelection = [];
        removed.forEach(function (critter) {
            critter.setSelected(false);
        });
        changeListener.notify([], removed);
    };

    /**
     * Returns whether nothing is selected.
     */
    Selection.isEmpty = function() {
        return currentSelection.length == 0;
    };

    var selectedIds; // This'll store the id's of selected party members from the savegame

    function load(payload) {
        // We'll have to store this temporarily to post-process it later on.
        selectedIds = payload.currentSelection;
    }

    function save(payload) {
        // We'll save the mobile id's instead of the mobile objects to prevent object duplication.
        payload.currentSelection = currentSelection.map(function(critter) {
            return critter.id;
        });
    }

    function afterLoad() {
        if (!selectedIds) {
            print("Calling after-load callback without having a selection in the savegame.");
            return;
        }

        currentSelection = [];
        selectedIds.forEach(function (id) {
            var mobile = Maps.findMobileById(id);
            if (mobile) {
                currentSelection.push(mobile);
                mobile.setSelected(true);
            } else {
                print("Cannot find mobile with id " + id + " to re-select it after savegame loaded.");
            }
        });
        selectedIds = [];
        changeListener.notify([], []); // Do an empty notification
    }

    StartupListeners.add(function() {
        SaveGames.addLoadingListener(load);
        SaveGames.addSavingListener(save);
        SaveGames.addLoadedListener(afterLoad);
    })

})();
