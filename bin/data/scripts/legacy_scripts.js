/**
 * Manages legacy scripts that have been converted from scr/*.py via an automated conversion
 * process.
 */
var LegacyScripts = {};

(function() {
    var scriptMapping;

    var scriptCache = {};

    var GameFacade = {

    };

    var LegacyScriptPrototype = {
        OBJ_HANDLE_NULL: null,

        SKIP_DEFAULT: true,

        RUN_DEFAULT: false,

        game: GameFacade,

        anyone: function(creatures, command, id) {
            print("Checking whether " + creatures + " have " + command + " " + id);
            return false; // TODO: Implement
        },

        /**
         * Checks a conversation guard using this script as the "this" context.
         * @param npc The NPC participating in the conversation.
         * @param pc The PC participating in the conversation.
         * @returns True if the guard is fulfilled, false otherwise.
         */
        checkGuard: function(npc, pc, guard) {
            return eval('(' + guard + ')');
        }
    };

    function loadScript(id) {
        var filename = scriptMapping[id];

        if (filename === undefined) {
            return undefined;
        }

        try {
            var result = eval('(' + readFile(filename) + ')');
            result.__proto__ = LegacyScriptPrototype;
            return result;
        } catch(err) {
            print("Unable to read legacy script file " + filename);
            throw err;
        }
    }

    /**
     * Retrieves a legacy script object.
     * @param id The id of the legacy script.
     * @returns A legacy script object or undefined if the id is unknown.
     */
    function getScript(id) {
        var obj = scriptCache[id];

        if (obj === undefined) {
            obj = loadScript(id);
            scriptCache[id] = obj;
        }

        return obj;
    }

    function checkGuards(npc, pc, answer) {
        var intCheck = answer.intelligence;

        if (intCheck !== undefined && intCheck < 0) {
            return false; // TODO: implement this. right now: always choose the high option.            
        }

        var guard = answer.guard;
        if (guard !== undefined) {
            var script = getScript(npc.obj.OnDialog.script);
            var result = script.checkGuard(npc, pc, guard);
            print("Checking guard: " + guard + " Result: " + result);
            return result;
        }
        return true;
    }

    var ObjectWrapper = function(obj) {
        if (!(this instanceof ObjectWrapper)) {
            print("You must use new ObjectWrapper instead of ObjectWrapper(...)!");
        }
        if (!obj) {
            print("Constructing an object wrapper for a null-object: Very bad idea.");
        }
        this.obj = obj;
    };

    /**
     * Checks if the NPC wrapped by this object has met the given player character yet.
     * @param character A player character.
     */
    ObjectWrapper.prototype.has_met = function(character) {
        return false; // TODO: No real implementation yet.
    };

    /**
     * Returns a list of the character in this characters group. (Have to check, may need to be wrapped)
     */
    ObjectWrapper.prototype.group_list = function() {
        return [this.obj]; // TODO: Implement
    };

    /**
     * Returns the current value of a statistic.
     * @param stat The statistic to return.
     */
    ObjectWrapper.prototype.stat_level_get = function(stat) {
        print("Retrieving statistic: " + stat);
        return 0;
    };

    /**
     * Returns the leader of an NPC. This seems to be an odd function to call.
     */
    ObjectWrapper.prototype.leader_get = function() {
        return null;
    };

    /**
     * This should return the current money-level for a player character in coppers (?)
     */
    ObjectWrapper.prototype.money_get = function() {
        return 10000;
    };

    /**
     * Returns whether the PC cannot accept more followers.
     */
    ObjectWrapper.prototype.follower_atmax = function() {
        return false;
    };

    /**
     * Changes the wealth of the party.
     * @param delta The amount of money to adjust. This may be negative or positive.
     */
    ObjectWrapper.prototype.money_adj = function(delta) {
        if (delta < 0)
            print("Taking " + delta + " money from the player.");
        else
            print("Giving " + delta + " money to the player.");
    };

    /**
     * Returns the effective rank of a skill (including bonuses) given a target.
     *
     * @param against Against who is the skill check made.
     * @param skill The skill identifier.
     */
    ObjectWrapper.prototype.skill_level_get = function(against, skill) {
        return 5;
    };

    /**
     * Opens the dialog interface and pauses the game.
     * @param pc The player character that is talking to the NPC.
     * @param line The dialog line to start on.
     */
    ObjectWrapper.prototype.begin_dialog = function(pc, line) {
        var npc = this; // Used by the val scripts
        print("Starting conversation with " + pc + " on line " + line);

        var dialogId = this.obj.OnDialog.script;

        var dialog = LegacyDialog.get(dialogId);

        if (!dialog) {
            print("Dialog not found: " + dialogId);
            return;
        }

        line = parseInt(line); // Ensure line is an integer

        var lastLine = Math.floor(line / 10) * 10 + 10;
        var answers = [];
        for (var i = line + 1; i < lastLine; ++i) {
            var answerLine = dialog[i];
            if (answerLine) {
                // Check the guard
                if (!checkGuards(this, pc, answerLine))
                    continue;

                answers.push({
                    id: i,
                    text: answerLine.text
                });
            }
        }

        var npcLine = dialog[line];

        if (npcLine.action) {
            print("Performing action: " + npcLine.action);
            eval(npcLine.action);
        }

        var voiceOver = null;

        /**
         * Guards on NPC lines are actually sound-ids
         */
        if (npcLine.guard) {
            var filename = 'sound/speech/';
            if (dialogId < 10000)
                filename += '0';
            if (dialogId < 1000)
                filename += '0';
            if (dialogId < 100)
                filename += '0';
            if (dialogId < 10)
                filename += '0';
            filename += dialogId + '/v' + npcLine.guard + '_m.mp3';

            print("Playing sound: " + filename);
            voiceOver = gameView.audioEngine.playSoundOnce(filename, SoundCategory_Effect);
        }

        var conversationDialog = gameView.addGuiItem("interface/Conversation.qml");
        conversationDialog.text = npcLine.text;
        conversationDialog.portrait = getPortrait(this.obj.portrait, Portrait_Medium);
        conversationDialog.answers = answers;
        var obj = this;
        conversationDialog.answered.connect(this, function(line) {
            conversationDialog.deleteLater();

            if (voiceOver) {
                print("Stopping previous voice over.");
                voiceOver.stop();
            }

            var action = dialog[line].action;

            if (action) {
                print("Performing action: " + action);
                eval(action);
            }

            var nextId = dialog[line].nextId;

            if (nextId) {
                print("Showing dialog for next id: " + nextId);
                obj.begin_dialog(pc, nextId);
            }
        });
    };

    /**
     * Loads the mapping of ids to legacy script files.
     */
    LegacyScripts.load = function() {
        scriptMapping = eval('(' + readFile('scripts.js') + ')');

        // This counting method may be inaccurate, but at least it gives us some ballpark estimate
        var count = 0;
        for (var k in scriptMapping)
            count++;
        print("Loaded " + count + " legacy scripts.");
    };

    /**
     * Triggers the OnDialog event for a legacy script.
     * @param attachedScript The legacy script. This is a JavaScript object with at least a 'script' property giving
     *                 the id of the legacy script.
     * @param attachedTo The object the script is attached to.
     */
    LegacyScripts.OnDialog = function(attachedScript, attachedTo) {
        var script = getScript(attachedScript.script);

        if (!script) {
            print("Unknown legacy script: " + attachedScript.script);
            return false;
        }

        var result = script.san_dialog(new ObjectWrapper(attachedTo), new ObjectWrapper(attachedTo));

        // TODO: Translate return code

        return result;
    }
})();
