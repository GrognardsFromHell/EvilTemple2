/**
 * Manages legacy scripts that have been converted from scr/*.py via an automated conversion
 * process.
 */
var LegacyScripts = {};

(function() {
    var scriptMapping;

    var scriptCache = {};

    var GameFacade = {
        /*
         Returns the number of members in the party.
         This only seems to extend to the players in the party.
         */
        party_size: function() {
            return Party.getPlayers().length;
        },

        /**
         * Opens the Party Pool interface to add/remove members from the party.
         */
        party_pool: function() {
            var text = 'Should open party pool now.';
            gameView.scene.addTextOverlay(gameView.worldCenter(), text, [0.9, 0.0, 0.0, 0.9]);
        },

        /**
         * Changes the current map.
         */
        fade_and_teleport: function(unk1, unk2, unk3, mapId, worldX, worldY) {
            var newPosition = [worldX * 28.2842703, 0, worldY * 28.2842703]; // this should probably go into the converter
            Maps.goToMap(mapId, newPosition);
        }
    };

    var LegacyScriptPrototype = {
        SKIP_DEFAULT: true,

        RUN_DEFAULT: false,

        game: GameFacade,

        anyone: function(creatures, command, id) {
            print("Checking whether " + creatures + " have " + command + " " + id);
            return false; // TODO: Implement
        },

        find_npc_near: function(critter, nameId) {

            var pos = critter.obj.position;

            print("Trying to find NPC with name " + nameId + " (" + translations.get('mes/description/' + nameId) + ") near " + pos);

            return null;

        },

        create_item_in_inventory: function(prototypeId, receiver) {
            print("Creating item with prototype " + prototypeId + " for " + receiver.obj.id);
        },

        /**
         * Checks a conversation guard using this script as the "this" context.
         * @param npc The NPC participating in the conversation.
         * @param pc The PC participating in the conversation.
         * @returns True if the guard is fulfilled, false otherwise.
         */
        checkGuard: function(npc, pc, guard) {
            return eval('(' + guard + ')');
        },

        /**
         * Performs an action with this script as the 'this' context.
         * @param npc The NPC participating in the conversation.
         * @param pc The PC participating in the conversation.
         * @param action The action to perform. This must be valid javascript code.
         */
        performAction: function (npc, pc, action) {
            // Perform in the context of the dialog script
            var proxy = {
                performAction: function(npc, pc) {
                    eval(action);
                }
            };
            proxy.__proto__ = this; // This forces the script to be accessible via the "this" variable
            print("Performing action: " + action);
            proxy.performAction(npc, pc);
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

    /**
     * Returns the player character that caused a legacy event, if no further information is available.
     *
     * Right now this is the first member of the party.
     */
    function getTriggerer() {
        // TODO: Later on, take selection into account. Right now, use first partymember
        return Party.getPlayers()[0];
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
        return this.obj.hasBeenTalkedTo;
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
        return Party.money.getTotalCopper();
    };

    /**
     * Returns whether the PC cannot accept more followers.
     */
    ObjectWrapper.prototype.follower_atmax = function() {
        return false;
    };

    /**
     * Adds a new follower to the party, if there's still room.
     * @param npc The npc to add to the party.
     */
    ObjectWrapper.prototype.follower_add = function(npc) {
        if (!npc || !npc.obj) {
            print("Passed null object to follower_add.");
            return false;
        }
        return Party.addFollower(npc.obj);
    },

        /**
         * Changes the wealth of the party.
         * @param delta The amount of money to adjust. This may be negative or positive.
         */
            ObjectWrapper.prototype.money_adj = function(delta) {
                if (delta < 0)
                    print("Taking " + Math.abs(delta) + " money from the player.");
                else
                    print("Giving " + delta + " money to the player.");
                Party.money.addCopper(delta);
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
     * This method causes NPCs to rummage through their inventory and equip all the items
     * they can (the best of their respective type).
     */
    ObjectWrapper.prototype.item_wield_best_all = function() {
        // TODO: Implement
    };

    // The current dialog UI if it's open
    var conversationDialog = null;

    /**
     * Opens the dialog interface and pauses the game. This should only be called on player characters.
     *
     * @param npc The NPC that should start the conversation.
     * @param line The dialog line to start on.
     */
    ObjectWrapper.prototype.begin_dialog = function(npc, line) {
        if (conversationDialog) {
            conversationDialog.deleteLater();
            conversationDialog = null;
        }

        var pc = this; // Used by the val scripts
        print("Starting conversation with " + npc.obj.id + " on line " + line);

        var dialogId = npc.obj.OnDialog.script;

        var dialog = LegacyDialog.get(dialogId);
        var script = getScript(dialogId);

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
                if (!checkGuards(npc, pc, answerLine))
                    continue;

                answerLine = LegacyDialog.transformLine(dialog, answerLine, npc.obj, pc.obj);

                answers.push({
                    id: i,
                    text: answerLine.text
                });
            }
        }

        var npcLine = dialog[line];

        if (npcLine.action) {
            script.performAction(npc, pc, npcLine.action);
        }

        npcLine = LegacyDialog.transformLine(dialog, npcLine, npc.obj, pc.obj);

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

        conversationDialog = gameView.addGuiItem("interface/Conversation.qml");
        conversationDialog.npcText = (pc.obj.gender == 'female' && npcLine.femaleText) ? npcLine.femaleText : npcLine.text;
        conversationDialog.npcName = npc.obj.getName(true);
        conversationDialog.portrait = getPortrait(npc.obj.portrait, Portrait_Medium);
        conversationDialog.answers = answers;

        conversationDialog.answered.connect(this, function(line) {
            if (conversationDialog) {
                conversationDialog.deleteLater();
                conversationDialog = null;
            }

            if (voiceOver) {
                print("Stopping previous voice over.");
                voiceOver.stop();
            }

            var action = dialog[line].action;

            if (action) {
                script.performAction(npc, pc, action);
            }

            var nextId = dialog[line].nextId;

            if (nextId) {
                print("Showing dialog for next id: " + nextId);
                pc.begin_dialog(npc, nextId);
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

        return script.san_dialog(new ObjectWrapper(attachedTo), new ObjectWrapper(getTriggerer()));
    };

    /**
     * Triggers the OnJoin event for a legacy script.
     *
     * @param attachedScript The legacy script. This is a JavaScript object with at least a 'script' property giving
     *                 the id of the legacy script.
     * @param attachedTo The object the script is attached to.
     */
    LegacyScripts.OnJoin = function(attachedScript, attachedTo) {
        var script = getScript(attachedScript.script);

        if (!script) {
            print("Unknown legacy script: " + attachedScript.script);
            return false;
        }

        return script.san_join(new ObjectWrapper(attachedTo), new ObjectWrapper(getTriggerer()));
    };

    /**
     * Triggers the OnDisband event for a legacy script.
     *
     * @param attachedScript The legacy script. This is a JavaScript object with at least a 'script' property giving
     *                 the id of the legacy script.
     * @param attachedTo The object the script is attached to.
     */
    LegacyScripts.OnDisband = function(attachedScript, attachedTo) {
        var script = getScript(attachedScript.script);

        if (!script) {
            print("Unknown legacy script: " + attachedScript.script);
            return false;
        }

        return script.san_disband(new ObjectWrapper(attachedTo), new ObjectWrapper(getTriggerer()));
    };

    /**
     * Triggers the OnUse event for a legacy script.
     *
     * @param attachedScript The legacy script. This is a JavaScript object with at least a 'script' property giving
     *                 the id of the legacy script.
     * @param attachedTo The object the script is attached to.
     */
    LegacyScripts.OnUse = function(attachedScript, attachedTo) {
        var script = getScript(attachedScript.script);

        if (!script) {
            print("Unknown legacy script: " + attachedScript.script);
            return false;
        }

        return script.san_use(new ObjectWrapper(attachedTo), new ObjectWrapper(getTriggerer()));
    };    

})();
