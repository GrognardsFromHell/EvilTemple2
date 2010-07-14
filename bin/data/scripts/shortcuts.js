
/**
 * Keyboard constants.
 */
var Keys = {
    F10: 0x01000039
};

/**
 * Manages keyboard shortcuts.
 */
var Shortcuts = {};

/**
 * Hook called from C++ code to notify us about a key event.
 * @param event The event parameter.
 */
function keyPressed(event) {
    Shortcuts.pressed(event.key);
}

(function() {

    var handlers = {};

    Shortcuts.register = function(key, handler) {
        handlers[key] = handler;
    };

    Shortcuts.pressed = function(key) {
        var handler = handlers[key];
        if (handler !== undefined) {
            handler(key);
        }
    };

})();
