/**
 * Handles game-time dependent events.
 *
 * Examples for this: Calmert will make Cavanaugh a beggar in 24 ingame hours.
 * These "timers" will also be persisted into the savegame.
 */
var TimedEvents = {};

(function() {

    // This list must keep sorted
    var timers = [];

    /**
     * Adds a timed event.
     * @param timeFromNow The time from now when the event will be triggered in game seconds.
     * @param callback The callback function that will be called when the event expires. Please note,
     *                  that the environment around the function will not be preserved, in case it is a closure call.
     *                  It is recommended to pass the *name* of a function here, so it can be properly called after the
     *                  savegame has been loaded.
     * @param args The arguments for the callback function. Special persistence rules apply here.
     *                  Functions cannot be persisted. Game objects will in general be converted to their
     *                  unique GUID, game objects that have no id are not valid as arguments. Objects in general are
     *                  valid arguments, but object <i>identity</i> will not be maintained. The callback may be given
     *                  a different object, with the same properties.
     */
    TimedEvents.add = function(timeFromNow, callback, args) {
        var ref = GameTime.getReference();
        ref.addTime(timeFromNow);

        if (callback instanceof Function)
            throw "You must not pass a Function as the callback. Pass a string instead.";

        // TODO: Validate arguments

        timers.push({
            expireTime: ref,
            callback: callback,
            args: args
        });

        print("Added a timer with expiretime " + ref + " Callback: " + callback);
    };

    function callTimer(callback, args) {
        var callbackFunc = eval(callback);

        if (!callbackFunc || !(callbackFunc instanceof Function)) {
            print("UNABLE TO CALL TIMER: " + callback);
            return;
        }

        callbackFunc.apply(null, args);
    }

    function processEvents() {
        for (var i = 0; i < timers.length; ++i) {
            if (timers[i].expireTime.isInThePast()) {
                print("Timer expired, calling " + timers[i].callback);
                callTimer(timers[i].callback, timers[i].args);
                timers.splice(i, 1);
                i--; // don't accidentally skip a timer
            }
        }
    }

    function persistObject(obj) {
        if (obj instanceof Array) {
            return obj.map(persistObject);
        } else if (typeof(obj) === 'object') {
            // Convert game objects to their ids.
            if (obj.type !== undefined && obj.id !== undefined) {
                return { 'objectId': obj.id };
            }

            var result = {};
            for (var k in obj) {
                if (obj.hasOwnProperty(k))
                    result[k] = persistObject(obj[k]);
            }
            return result;
        } else {
            return obj;
        }
    }

    function unpersistObject(obj) {
        if (obj instanceof Array) {
            for (var i = 0; i < obj.length; ++i)
                obj[i] = unpersistObject(obj[i]);
        } else if (typeof(obj) === 'object') {
            // Convert game objects to their ids.
            if (obj.objectId !== undefined) {
                return Maps.findMobileById(obj.objectId);
            }

            for (var k in obj) {
                if (obj.hasOwnProperty(k))
                    obj[k] = unpersistObject(obj[k]);
            }

        }

        return obj;
    }

    function save(payload) {
        var events = [];

        timers.forEach(function (timer) {
            events.push({
                expireTime: timer.expireTime,
                callback: timer.callback,
                args: timer.args.map(persistObject)
            });
        });

        payload.timedEvents = events;
    }

    var loadedEvents;

    function load(payload) {
        timers = [];

        if (!payload.timedEvents) {
            loadedEvents = undefined;
            return;
        }

        loadedEvents = payload.timedEvents;
    }

    function postLoad() {
        if (!loadedEvents)
            return;

        loadedEvents.forEach(function (event) {
            event.expireTime = new TimeReference(event.expireTime.secondOfTheYear, event.expireTime.year);
            unpersistObject(event.args);
        });

        timers = loadedEvents;
        loadedEvents = undefined;

        print("Loaded " + timers.length + " timers from the savegame.");
    }

    StartupListeners.add(function() {
        GameTime.addTimeChangedListener(processEvents);
        SaveGames.addLoadingListener(load);
        SaveGames.addSavingListener(save);
        SaveGames.addLoadedListener(postLoad);
    });

})();
