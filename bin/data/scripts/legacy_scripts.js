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
            return script.checkGuard(npc, pc, guard);
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
     * Opens the dialog interface and pauses the game.
     * @param talkingTo The character that is talking to the NPC.
     * @param line The dialog line to start on.
     */
    ObjectWrapper.prototype.begin_dialog = function(talkingTo, line) {
        print("Starting conversation with " + talkingTo + " on line " + line);

        var dialogId = this.obj.OnDialog.script;

        var dialog = LegacyDialog.get(dialogId);

        if (!dialog) {
            print("Dialog not found: " + dialogId);
            return;
        }

        line = parseInt(line); // Ensure line is an integer

        print("First answer: " + (line + 1));
        var lastLine = Math.floor(line / 10) * 10 + 10;
        print("Last answer: " + lastLine);

        var answers = [];
        for (var i = line + 1; i < lastLine; ++i) {
            var answerLine = dialog[i];
            if (answerLine) {
                // Check the guard
                if (!checkGuards(this, talkingTo, answerLine))
                    continue;

                answers.push({
                    id: i,
                    text: answerLine.text
                });
            }
        }

        var npcLine = dialog[line];

        if (npcLine.action)
            print("Perform action: " + npcLine.action);

        var conversationDialog = gameView.addGuiItem("interface/Conversation.qml");
        conversationDialog.text = npcLine.text;
        conversationDialog.portrait = getPortrait(this.obj.portrait, Portrait_Medium);
        conversationDialog.answers = answers;
        var obj = this;
        conversationDialog.answered.connect(this, function(line) {
            conversationDialog.deleteLater();

            var action = dialog[line].action;

            if (action) {
                print("Performing action: " + action);
            }

            var nextId = dialog[line].nextId;

            if (nextId) {
                print("Showing dialog for next id: " + nextId);
                obj.begin_dialog(talkingTo, nextId);
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
