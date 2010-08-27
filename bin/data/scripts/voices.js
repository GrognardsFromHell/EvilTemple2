/**
 * This object manages the various player voice-styles.
 */
var Voices = {};

var Voice = function() {
};

Voice.Acknowledge = 'acknowledge';
Voice.Deny = 'deny';
Voice.Death = 'death';
Voice.Encumbered = 'encumbered';
Voice.CriticalHp = 'criticalHp';
Voice.SeesDeath = 'seesDeath';
Voice.Combat = 'combat';
Voice.CriticalHitByParty = 'criticalHitByParty';
Voice.CriticalHitOnParty = 'criticalHitOnParty';
Voice.CriticalMissByParty = 'criticalMissByParty';
Voice.FriendlyFire = 'friendlyFire';
Voice.ValuableLoot = 'valuableLoot';
Voice.BossMonster = 'bossMonster';
Voice.Bored = 'bored';

Voice.prototype.playGeneric = function(type) {
    var lines = this.generic[type];
    if (!lines)
        throw "Unknown generic voice type: " + type;
    if (lines.length == 0)
        return;

    var i = randomRange(0, lines.length - 1);
    var line = lines[i];

    print(line.text);
    if (line.sound) {
        var filename = this.basePath + line.sound;
        gameView.audioEngine.playSoundOnce(filename, SoundCategory_Effect);
    }
};

(function() {

    var voicesById = {};
    var voices = [];

    Voices.register = function(voiceObj) {
        print("Registering voice " + voiceObj.id + " (" + voiceObj.name + ")");

        if (voiceObj.id === null || voiceObj.id === undefined)
            throw "Voice object has no id property.";

        if (voicesById[voiceObj.id])
            throw "Voice with id " + voiceObj.id + " is already registered.";

        var actualObj = new Voice;
        for (var k in voiceObj) {
            if (voiceObj.hasOwnProperty(k))
                actualObj[k] = voiceObj[k];
        }

        voicesById[voiceObj.id] = actualObj;
        voices.push(actualObj);
    };

    Voices.getById = function(id) {
        return voicesById[id];
    };

    Voices.getAll = function() {
        return voices.slice(0);
    };

    function loadVoices() {
        print("Loading voices...");
        eval('(' + readFile('voices.js') + ')').forEach(function (voice) {
            Voices.register(voice);
        });
    }

    StartupListeners.add(loadVoices, 'toee-voices', []);

})();
