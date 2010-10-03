var jumppoints = {};

var editMode = false;

var tutorialMode = false; // Indicates that the tutorial is running.

var sounds;

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
