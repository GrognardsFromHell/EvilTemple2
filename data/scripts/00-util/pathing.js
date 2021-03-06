/**
 * Describes a path between two points in the world.
 * The path may be updated and traversed.
 */
var Path = function(object, to) {
    if (!(this instanceof Path))
        throw "Only use the Path constructor.";
    this.update(object, to);
};

(function() {

    /**
     * Returns whether this path is empty.
     */
    Path.prototype.isEmpty = function() {
        return this.segments.length == 0;
    };

    /**
     * Forces this path to be updated.
     * @param object The object that should be moved along the path. Used to access position and radius.
     * @param to The target point for the path.
     */
    Path.prototype.update = function(object, to) {
        this.from = object.position;
        this.radius = object.radius;

        this.segments = [];

        var points;

        if (to instanceof Array) {
            print("Moving using array");
            points = Maps.currentMap.findPath(object, to);
        } else {
            points = Maps.currentMap.findPathIntoRange(object, to, 25);
        }

        for (var i = 0; i < points.length - 1; ++i) {
            var d = V3.sub(points[i + 1], points[i]);
            var l = V3.length(d);
            V3.normalize(d, d); // We want a unit-length directional vector
            this.segments.push({
                vector: d,
                length: l
            });
        }
    };

    /**
     * Returns the current directional vector or undefined if this path is empty.
     */
    Path.prototype.currentDirection = function() {
        if (this.segments.length > 0)
            return this.segments[0].vector;
        else
            return undefined;
    };

    /**
     * Advances along this path by a given distance. The function will return the new
     * position along the path.
     *
     * @param distance The distance by which to advance.
     */
    Path.prototype.advance = function(distance) {
        while (distance > 0 && this.segments.length > 0) {
            var segment = this.segments[0];

            // We only partially consume this segment
            if (distance < segment.length) {
                segment.length -= distance;
                var v = V3.scale(segment.vector, distance);
                return V3.add(this.from, v, this.from);
            }

            // We will consume this segment in it's entirety
            V3.scale(segment.vector, segment.length, segment.vector); // We can reuse this as it'll be removed
            V3.add(this.from, segment.vector, this.from);
            this.segments.splice(0, 1);
        }

        return this.from;
    };

})();