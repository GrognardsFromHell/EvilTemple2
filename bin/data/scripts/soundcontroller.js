/**
 * Controls background music and ambient sounds.
 */
var SoundController = {};

(function() {

    var soundSchemes = readJson('soundschemes.js');

    var activeSchemes = [];

    var activeSounds = [];

    var heartbeatInterval = 2000;

    function isActive(record) {
        var currentHour = GameTime.getHourOfDay();

        if (record.time) {
            if (currentHour < record.time.from || currentHour > record.time.to) {
                print("Skipping music " + record.filename + " since it's not in the time-range.");
                return false;
            }
        }

        return true;
    }

    /**
     * This selects an ambient sound and plays it.
     * The exact logic is still unknown.
     */
    function ambientSoundHeartbeat() {
        var i, j, scheme, sound, chance, handle;

        for (i = 0; i < activeSchemes.length; ++i) {
            scheme = activeSchemes[i];

            for (j = 0; j < scheme.ambientSounds.length; ++j) {
                sound = scheme.ambientSounds[j];

                chance = sound.frequency / 200;
                if (Math.random() < chance) {
                    handle = gameView.audioEngine.playSoundOnce(sound.filename, SoundCategory_Ambience);
                    handle.volume = sound.volume / 100;
                }
            }
        }

        gameView.addVisualTimer(heartbeatInterval, ambientSoundHeartbeat);
    }

    /**
     * Plays the combat intro sound for the current scheme, or if no custom one is defined,
     * plays the fallback intro.
     */
    function playCombatIntro() {
        var filename = 'sound/music/combatintro.mp3';

        activeSchemes.some(function(scheme) {
            if (scheme.combatIntro) {
                filename = scheme.combatIntro.filename;
                return true;
            }
            return false;
        });

        // In theory a scheme could unset the combat intro for whatever reason.
        if (filename) {
            gameView.audioEngine.playSoundOnce(filename, SoundCategory_Music);
        }
    }

    SoundController._playCombatIntro = playCombatIntro;

    function removeInactiveSounds() {
        var i, sound, remove;

        print("Trying to remove inactive sounds (" + activeSounds.length + ")");

        for (i = 0; i < activeSounds.length; ++i) {
            // See if the sound still applies.
            sound = activeSounds[i];
            remove = false;

            print("Checking " + sound.filename);

            if (activeSchemes.indexOf(sound.scheme) == -1) {
                print("Sound's scheme is no longer active.");
                remove = true;
            } else if (!isActive(sound)) {
                print("Sound itself is no longer active (i.e. time)");
                remove = true;
            }

            if (remove) {
                print("Removing sound " + sound.filename);
                try {
                    sound.handle.stop();
                } catch (e) {
                }
                activeSounds.splice(i, 1);
                sound = undefined;
                i--;
            }
        }
    }

    function updatePlaying() {
        var i, j, scheme, music, handle, sound;

        removeInactiveSounds();

        for (i = 0; i < activeSchemes.length; ++i) {
            scheme = activeSchemes[i];

            for (j = 0; j < scheme.backgroundMusic.length; ++j) {
                music = scheme.backgroundMusic[j];

                if (!isActive(music))
                    continue;

                print("Music: " + music.filename);
                sound = gameView.audioEngine.readSound(music.filename);
                handle = gameView.audioEngine.playSound(sound, SoundCategory_Music, true);
                if (music.volume) {
                    print("Setting music volume to " + music.volume + "%");
                    handle.volume = music.volume / 100;
                }

                var activeSound = {
                    handle: handle,
                    scheme: scheme
                };
                activeSound.__proto__ = music;
                activeSounds.push(activeSound);
            }
        }
    }

    /**
     * Activates a set of sound schemes.
     * All active schemes will be deactivated first.
     *
     * @param schemes The schemes to activate.
     */
    SoundController.activate = function(schemes) {
        activeSchemes = [];

        schemes.forEach(function (name) {
            var scheme = soundSchemes[name];
            if (!scheme) {
                print("Skipping unknown sound scheme: " + name);
            } else {
                activeSchemes.push(scheme);
            }
        });

        updatePlaying();
    };

    /**
     * Deactivates all active sound schemes.
     */
    SoundController.deactivate = function() {
        activeSounds.forEach(function (sound) {
            sound.handle.stop();
        });

        activeSounds = [];
        activeSchemes = [];
        gc();
    };

    // Update music when the hour of the day changes.
    StartupListeners.add(function() {
        GameTime.addHourChangedListener(updatePlaying);
        gameView.addVisualTimer(heartbeatInterval, ambientSoundHeartbeat);
        // TODO: Combat Enter/Leave needs to trigger switch to combat music

        Combat.addCombatStartListener(playCombatIntro);
    });

})();
