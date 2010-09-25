var jumppoints = {};

var editMode = false;

var tutorialMode = false; // Indicates that the tutorial is running.

var sounds;

var worldClickCallback = null;

function selectWorldTarget(callback) {
    worldClickCallback = callback;
}

function setupWorldClickHandler() {
    var firstClick = undefined;

    gameView.worldClicked.connect(function(event, worldPosition) {
        if (worldClickCallback != null) {
            var callback = worldClickCallback;
            worldClickCallback = null;
            callback(worldPosition);
            return;
        }

        // In case combat is active, all world click events are forwarded to the combat ui
        if (Combat.isActive()) {
            CombatUi.worldClicked(event, worldPosition);
            return;
        }

        // Clear the selection if right-clicked
        if (event.button == Mouse.RightButton) {
            Selection.clear();
            return;
        }

        var color;
        var material = Maps.currentMap.material(worldPosition);

        if (!Selection.isEmpty()) {
            Selection.get().forEach(function(mobile) {
                walkTo(mobile, worldPosition);
            });
            return;
        }

        var text;
        if (firstClick === undefined) {
            text = "1st @ " + Math.floor(worldPosition[0]) + "," + Math.floor(worldPosition[2]) + " (" + material + ")";
            gameView.scene.addTextOverlay(worldPosition, text, [0.9, 0.9, 0.9, 0.9]);
            firstClick = worldPosition;
        } else {
            var inLos = gameView.sectorMap.hasLineOfSight(firstClick, worldPosition);
            if (inLos) {
                color = [0.2, 0.9, 0.2, 0.9];
            } else {
                color = [0.9, 0.2, 0.2, 0.9];
            }

            text = "2nd @ " + Math.floor(worldPosition[0]) + "," + Math.floor(worldPosition[2]) + " (" + material + ")";
            gameView.scene.addTextOverlay(worldPosition, text, color);

            var path = gameView.sectorMap.findPath(firstClick, worldPosition);

            print("Found path of length: " + path.length);

            var sceneNode = gameView.scene.createNode();

            for (var i = 0; i < path.length; ++i) {
                if (i + 1 < path.length) {
                    var line = new LineRenderable(gameView.scene);
                    line.addLine(path[i], path[i + 1]);
                    sceneNode.attachObject(line);
                }

                gameView.scene.addTextOverlay(path[i], 'X', [1, 0, 0], 1);
            }

            gameView.addVisualTimer(5000, function() {
                print("Removing scene node.");
                gameView.scene.removeNode(sceneNode);
            });

            firstClick = undefined;
        }
    });

    gameView.worldDoubleClicked.connect(function(event, worldPosition) {
        print("Doubleclicked: " + objectToString(event));
        gameView.centerOnWorld(worldPosition);
    });
}

var animEventGameFacade = {
    particles: function(partSysId, proxyObject) {
        var renderState = proxyObject.obj.getRenderState();

        if (!renderState || !renderState.modelInstance || !renderState.modelInstance.model) {
            print("Called game.particles (animevent) for an object without rendering state: " + proxyObject.obj.id);
            return;
        }

        var particleSystem = gameView.particleSystems.instantiate(partSysId);
        particleSystem.modelInstance = renderState.modelInstance;
        renderState.sceneNode.attachObject(particleSystem);
    },
    sound_local_obj: function(soundId, proxyObject) {
        var filename = sounds[soundId];
        if (filename === undefined) {
            print("Unknown sound id: " + soundId);
            return;
        }

        print("Playing sound " + filename);
        var handle = gameView.audioEngine.playSoundOnce(filename, SoundCategory_Effect);
        handle.setPosition(proxyObject.obj.position);
        handle.setMaxDistance(1500);
        handle.setReferenceDistance(150);
    },
    shake: function(a, b) {
        print("Shake: " + a + ", " + b);
    }
};

var footstepSounds = {
    'dirt': ['sound/footstep_dirt1.wav', 'sound/footstep_dirt2.wav', 'sound/footstep_dirt3.wav', 'sound/footstep_dirt4.wav'],
    'sand': ['sound/footstep_sand1.wav', 'sound/footstep_sand2.wav', 'sound/footstep_sand3.wav', 'sound/footstep_sand4.wav'],
    'ice': ['sound/footstep_snow1.wav', 'sound/footstep_snow2.wav', 'sound/footstep_snow3.wav', 'sound/footstep_snow4.wav'],
    'stone': ['sound/footstep_stone1.wav', 'sound/footstep_stone2.wav', 'sound/footstep_stone3.wav', 'sound/footstep_stone4.wav'],
    'water': ['sound/footstep_water1.wav', 'sound/footstep_water2.wav', 'sound/footstep_water3.wav', 'sound/footstep_water4.wav'],
    'wood': ['sound/footstep_wood1.wav', 'sound/footstep_wood2.wav', 'sound/footstep_wood3.wav', 'sound/footstep_wood4.wav']
};

var footstepCounter = 0;

var animEventAnimObjFacade = {
    footstep: function() {
        // TODO: Should we use Bip01 Footsteps bone here?
        var material = Maps.currentMap.material(this.obj.position);

        if (material === undefined)
            return;

        var sounds = footstepSounds[material];

        if (sounds === undefined) {
            print("Unknown material-type: " + material);
            return;
        }

        var sound = sounds[footstepCounter++ % sounds.length];

        var handle = gameView.audioEngine.playSoundOnce(sound, SoundCategory_Effect);
        handle.setPosition(this.obj.position);
        handle.setMaxDistance(1500);
        handle.setReferenceDistance(50);
    }
};

function handleAnimationEvent(type, content) {
    // Variable may be used by the eval call below.
    //noinspection UnnecessaryLocalVariableJS
    var game = animEventGameFacade;

    var anim_obj = {
        obj: this
    };
    anim_obj.__proto__ = animEventAnimObjFacade;

    /**
     * Type 1 seems to be used to signify the frame on which an action should actually occur.
     * Examle: For the weapon-attack animations, event type 1 is triggered, when the weapon
     * would actually hit the opponent, so a weapon-hit-sound can be emitted at exactly
     * the correct time.
     */
    if (type == 1) {
        if (this.goal) {
            this.goal.animationAction(this, content);
        }
        return;
    }

    if (type != 0) {
        print("Skipping unknown animation event type: " + type);
        return;
    }

    /*
     Python one-liners are in general valid javascript, so we directly evaluate them here.
     Eval has access to all local variables in this scope, so we can define game + anim_obj,
     which are the most often used by the animation events.
     */
    eval(content);
}
