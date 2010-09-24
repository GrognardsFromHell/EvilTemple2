
/**
 * Keyboard constants.
 * They come from: http://doc.qt.nokia.com/4.7-snapshot/qt.html#Key-enum
 */
var Keys = {
    F5: 0x01000034, 
    F10: 0x01000039,
    AsciiTilde: 0x7E
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
