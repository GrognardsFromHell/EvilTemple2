
var walkJobs = [];

var WalkJobs = {
    canceled: false,
    driven: 0,
    currentPathNode: 0,
    addTime: function(time) {
        if (this.canceled)
            return;
        var driven = this.speed * time;
        this.modelInstance.elapseDistance(driven);
        this.driven += driven;
        if (this.driven + driven > this.length) {
            driven = this.length - this.driven;
        }

        var pos = this.sceneNode.position;
        var dx = this.direction[0] * driven;
        var dy = this.direction[1] * driven;
        var dz = this.direction[2] * driven;

        this.sceneNode.position = new Vector4(pos.x + dx, pos.y + dy, pos.z + dz, 1);

        if (this.driven >= this.length) {
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
            this.init(this.path[this.currentPathNode], this.path[this.currentPathNode+1]);
        }

        var obj = this;
        gameView.addVisualTimer(30, function() {
            obj.addTime(0.03);
        });
    },
    init: function(from, to) {
        var x = to.x - from.x;
        var y = to.y - from.y;
        var z = to.z - from.z;

        var l = Math.sqrt((x * x) + (y * y) + (z * z));

        x /= l;
        y /= l;
        z /= l;

        print("Pathlength: " + l + " Direction: " + x + "," + y + "," + z);

        this.length = l;
        this.direction = [x,y,z];

        // Normal view-direction
        var nx = 0;
        var ny = 0;
        var nz = -1;

        var rot = Math.acos(nx * x + ny * y + nz * z);
        if (x > 0) {
            rot = - rot;
        }
        this.sceneNode.rotation = new Quaternion(0, Math.sin(rot / 2), 0, Math.cos(rot / 2));
    }
};

/*
    Implements walking between two points.
 */
function walkTo(obj, sceneNode, modelInstance, to) {

    for (var i = 0; i < walkJobs.length; ++i) {
        if (walkJobs[i].obj === obj) {
            walkJobs[i].canceled = true;
            walkJobs.splice(i, 1);
            --i;
        }
    }

    var path = gameView.sectorMap.findPath(sceneNode.position, to);

    if (path.length == 0)
        return false;

    var job = {
        obj: obj,
        sceneNode: sceneNode,
        modelInstance: modelInstance,
        path: path,
        speed: modelInstance.model.animationDps('unarmed_unarmed_run'), // Pixel per second
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
