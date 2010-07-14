var walkJobs = [];

var WalkJobs = {
    canceled: false,
    driven: 0,
    currentPathNode: 0,
    addTime: function(time) {
        try {
            if (this.canceled)
                return;

            var driven = this.speed * time;
            this.modelInstance.elapseDistance(driven);
            this.driven += driven;

            var completion = this.driven / this.length;

            if (completion < 1) {
                var pos = this.path[this.currentPathNode];
                var dx = this.direction[0] * completion;
                var dy = this.direction[1] * completion;
                var dz = this.direction[2] * completion;

                var position = [pos[0] + dx, pos[1] + dy, pos[2] + dz];
                this.sceneNode.position = position;
                this.obj.position = position;
            } else {
                this.currentPathNode += 1;
                if (this.currentPathNode + 1 >= this.path.length) {
                    this.modelInstance.stopAnimation();

                    for (var i = 0; i < walkJobs.length; ++i) {
                        if (walkJobs[i] === this) {
                            walkJobs.splice(i, 1);
                            --i;
                        }
                    }

                    return;
                }
                this.driven = 0;
                this.init(this.path[this.currentPathNode], this.path[this.currentPathNode + 1]);
            }

            var obj = this;
            gameView.addVisualTimer(30, function() {
                obj.addTime(0.03);
            });
        } catch (error) {
            print(error);
        }
    },
    init: function(from, to) {
        // Build a directional vector from the start pointing to the target
        var x = to[0] - from[0];
        var y = to[1] - from[1];
        var z = to[2] - from[2];

        var l = Math.sqrt((x * x) + (y * y) + (z * z));

        print("Pathlength: " + l + " Direction: " + x + "," + y + "," + z);

        this.length = l;
        this.direction = [x, y, z];

        // Normal view-direction
        var nx = 0;
        var ny = 0;
        var nz = -1;

        x /= l;
        y /= l;
        z /= l;

        var rot = Math.acos(nx * x + ny * y + nz * z);
        if (x > 0) {
            rot = - rot;
        }

        this.obj.rotation = rot;
        this.sceneNode.rotation = [0, Math.sin(rot / 2), 0, Math.cos(rot / 2)];
    }
};

function showPath(path)
{
    var sceneNode = gameView.scene.createNode();

    var line = new LineRenderable(gameView.scene);
    for (var i = 0; i < path.length; ++i) {
        if (i + 1 < path.length) {
            line.addLine(path[i], path[i + 1]);
        }
    }
    sceneNode.attachObject(line);

    gameView.addVisualTimer(5000, function() {
        print("Removing scene node.");
        gameView.scene.removeNode(sceneNode);
    });
}

/*
 Implements walking between two points.
 */
function walkTo(obj, to) {
    var renderState = obj.getRenderState();

    if (!renderState)
        return false;

    var sceneNode = renderState.sceneNode;
    var modelInstance = renderState.modelInstance;

    for (var i = 0; i < walkJobs.length; ++i) {
        if (walkJobs[i].obj === obj) {
            modelInstance.stopAnimation();
            walkJobs[i].canceled = true;
            walkJobs.splice(i, 1);
            --i;
        }
    }

    var path = gameView.sectorMap.findPath(sceneNode.position, to);

    if (path.length == 0)
        return false;

    showPath(path); // Debugging

    var job = {
        obj: obj,
        sceneNode: sceneNode,
        modelInstance: modelInstance,
        path: path,
        speed: modelInstance.model.animationDps('unarmed_unarmed_run') // Pixel per second
    };
    job.__proto__ = WalkJobs;
    job.init(path[0], path[1]);

    walkJobs.push(job);
    modelInstance.playAnimation('unarmed_unarmed_run', true);

    gameView.addVisualTimer(30, function() {
        job.addTime(0.03);
    });
    return true;
}
