/**
 * Controls background music and ambient sounds.
 */
var SoundController = {};

(function() {

    var soundSchemes = readJson('soundschemes.js');

    var activeSchemes = [];

    var activeHandles = [];

    function isActive(record) {
        var currentHour = GameTime.getHourOfDay();

        if (record.time) {
            if (currentHour < record.time.from || currentHour > record.time.to) {
                print("Skipping music " + record.filename + " since it's not in the time-range.");
                return true;
            }
        }

        return true;
    }

    function updatePlaying() {
        var i, j, scheme, music, handle, sound;

        for (i = 0; i < activeSchemes.length; ++i) {
            scheme = activeSchemes[i];

            for (j = 0; j < scheme.backgroundMusic.length; ++j) {
                music = scheme.backgroundMusic[j];

                if (!isActive(music))
                    continue;

                print("Music: " + music.filename);
                sound = gameView.audioEngine.readSound(music.filename);
                handle = gameView.audioEngine.playSound(sound, SoundCategory_Music, true);
                if (music.volume)
                    handle.volume = music.volume / 100;
                activeHandles.push(handle);
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
        this.deactivate();

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
        activeHandles.forEach(function (handle) {
            handle.stop();
        });

        activeHandles = [];
        activeSchemes = [];
        gc();
    };

})();
