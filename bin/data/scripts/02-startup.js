
var StartupListeners = {

    listeners: [],

    add: function(callback) {
        this.listeners.push(callback);
    },

    call: function() {
        this.listeners.forEach(function(callback) {
            callback.call();
        });
    }

};
