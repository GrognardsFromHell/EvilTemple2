
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
    Creates a quaternion that rotates around the Y axis by the given amount (in degrees).
 */
function rotationFromDegrees(degrees) {
    var radians = degrees * 0.0174532925199432;
    var cosRot = Math.cos(radians / 2);
    var sinRot = Math.sin(radians / 2);
    return [0, sinRot, 0, cosRot];
}
