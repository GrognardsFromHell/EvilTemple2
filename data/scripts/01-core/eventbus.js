/**
 * Provides a global event bus that can be used to wire components together.
 */
var EventBus = {
    listeners: {},
    descriptions: {}
};

/**
 * Registers a new event type.
 * @param type The event type identifier. This must also be used to notify and register listeners.
 * @param description A description of the event-type which can be used for documentation purposes.
 */
EventBus.registerEvent = function(type, description) {

    with (EventBus) {
        if (listeners[type])
            throw "Event type is already registered: " + type;

        listeners[type] = new ListenerQueue;
        descriptions[type] = description;
    }

};

EventBus.addListener = function(type, callback, thisObject) {

    with (EventBus) {
        if (!listeners[type])
            throw "Unknown event-type: " + type;

        listeners[type].append(callback, thisObject);
    }

};

EventBus.removeListener = function(type, callback, thisObject) {

    with (EventBus) {
        if (!listeners[type])
            throw "Unknown event type: " + type;

        listeners[type].remove(callback, thisObject);
    }

};

EventBus.notify = function(type) {

    with (EventBus) {

        if (!listeners[type])
            throw "Unknown event type: " + type;

        // The listeners will get everything other than the type passed through as arguments.
        var args = [];
        for (var i = 1; i < arguments.length; ++i)
            args.push(arguments[i]);

        listeners[type].notify.apply(listeners[type], args);

    }

};

var EventTypes = {
    GoalStarted: 'goal_started',
    GoalFinished: 'goal_finished',
    CritterDied: 'critter_died'
};

for (var k in EventTypes)
    EventBus.registerEvent(EventTypes[k]);
