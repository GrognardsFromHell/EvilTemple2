
/**
 * A queue of listeners that can be notified about events.
 */
var ListenerQueue = function() {
    this.listeners = [];
};

ListenerQueue.prototype.append = function(callback, thisObject) {
    this.listeners.push([callback, thisObject]);
};

ListenerQueue.prototype.remove = function(callback, thisObject) {
    for (var i = 0; i < this.listeners.length; ++i) {
        var listener = this.listeners[i];

        if (listener[0] === callback && listener[1] === thisObject) {
            this.listeners.splice(i, 1);
            return true;
        }
    }

    return false;
};

ListenerQueue.prototype.notify = function() {
    for (var i = 0; i < this.listeners.length; ++i) {
        this.listeners[i][0].apply(this.listeners[i][1], arguments);
    }
};
