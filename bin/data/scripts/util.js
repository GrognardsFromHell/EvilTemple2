
/*
    Reads a JSON file from disk (UTF-8 encoding) and returns the contained top-level object.
 */
function readJson(filename)
{
    var start = timerReference();

    var text = readFile(filename);
    var result = eval('(' + text + ')');

    var diff = timerReference() - start;
    print("Loaded " + filename + " in " + diff + " ms.");
    return result;
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
