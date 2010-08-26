/*
 Reads a JSON file from disk (UTF-8 encoding) and returns the contained top-level object.
 */
function readJson(filename) {
    var text = readFile(filename);
    var result;
    try {
        result = eval('(' + text + ')');
    } catch (e) {
        throw "Unable to load file " + filename + " due to a syntax error: " + e;
    }

    return result;
}

/**
 * Writes an object to disk using JSON.
 *
 * @param filename The filename to write to.
 * @param object The object to write.
 */
function writeJson(filename, object) {
    var json = JSON.stringify(object);
    writeFile(filename, json);
}

/**
 * Calculates a random integral number between a lower and an upper bound.
 *
 * @param min The lower bound (inclusive).
 * @param max The upper bound (inclusive).
 * @returns A random number x (integral), with x >= min && x <= max.
 */
function randomRange(min, max) {
    // TODO: Replace this with code that uses mersenne twister
    var result = Math.floor(Math.random() * (max - min + 1)) + min;

    print("random: " + min + "-" + max + ' = ' + result);

    return result;
}

/**
 Creates a quaternion that rotates around the Y axis by the given amount (in degrees).
 */
function rotationFromDegrees(degrees) {
    var radians = degrees * 0.0174532925199432;
    var cosRot = Math.cos(radians / 2);
    var sinRot = Math.sin(radians / 2);
    return [0, sinRot, 0, cosRot];
}

/**
 * Calculates the distance between two 3d points.
 *
 * @param pointA The first point.
 * @param pointB The second point.
 * @returns The distance in world units.
 */
function distance(pointA, pointB) {
    var dx = pointA[0] - pointB[0];
    var dy = pointA[1] - pointB[1];
    var dz = pointA[2] - pointB[2];

    var squaredDistance = dx * dx + dy * dy + dz * dz;

    return Math.sqrt(squaredDistance);
}

/**
 * Converts an object deeply into a string, by resolving objects and arrays.
 * @param value The object to convert.
 */
function objectToString(value) {
    var result = '{';

    for (var sk in value) {
        if (result != '{')
            result += ', ';
        result += sk + ': ' + value[sk];
    }

    result += '}';
    return result;
}

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

function assertTrue(actual, msg) {
    if (!actual)
        throw msg;
}

/**
 * Returns the modifier for a given ability value.
 * I.e. for 18 it returns +4, for 8 -1, etc.
 *
 * @param value The ability value.
 */
function getAbilityModifier(value) {
    return Math.floor((value - 10) / 2);
}

/**
 * Converts an angle from radians to degrees.
 * @param radians The angle in radians.
 */
function rad2deg(radians) {
    return (radians / Math.PI) * 180;
}

Array.prototype.equals = function(other) {
    if (!(other instanceof Array) || other.length != this.length)
        return false;

    for (var i = 0; i < other.length; ++i) {
        if (this[i] != other[i])
            return false;
    }

    return true;
};

/**
 * Gets a bonus as a string conforming to D20 convention for bonuses. This means that positive
 * bonuses have an explicit plus sign prepended.
 *
 * @param bonus The bonus to convert.
 */
function bonusToString(bonus) {
    return (bonus < 0) ? String.valueOf(bonus) : "+" + bonus;
}

