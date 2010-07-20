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
        fade_and_teleport: function(unk1, unk2, movieId, mapId, worldX, worldY) {

            var goToNewMap = function() {
                var newPosition = [worldX, 0, worldY]; // this should probably go into the converter

                var map = Maps.mapsById[mapId];
                if (map)
                    Maps.goToMap(map, newPosition);
                else
                    print("Unknown legacy map id: " + mapId);
            };

            if (movieId) {
                var movieMap = readJson('movies/movies.js');
                if (movieMap[movieId]) {
                    var movie = movieMap[movieId];
                    if (gameView.playMovie(movie.filename, goToNewMap))
                        return;
                } else {
                    print("Unknown movie id: " + movieId);
                }
            }

            goToNewMap();
        },

        /**
         * This needs to be updated before each dialog. Contains object wrappers for the party.
         */
        party: [],

        obj_list_vicinity: function(pos, type) {
            // Filter the map's mobile list.
            var result = Maps.currentMap.mobiles.filter(function (mobile) {
                if ((type == 'Critter' && mobile.type != 'NonPlayerCharacter' && mobile.type != 'PlayerCharacter')
                        || (type != 'Critter' && mobile.type != type)) {
                    return false; // Skip objects that are not of the requested type
                }

                // Check that the NPC is in the vicinity, whatever that means
                return distance(pos, mobile.position) <= 128;
            });

            result = result.map(function(obj) {
                return new CritterWrapper(obj);
            });

            return result;
        },

        /**
         * Shows flags on the townmap. Currently not implemented.
         *
         * @param mapId
         * @param flagId
         * @param enabled
         */
        map_flags: function(mapId, flagId, enabled) {
            if (enabled)
                print("Showing flagid " + flagId + " on map " + mapId);
            else
                print("Hiding flagid " + flagId + " on map " + mapId);
        },

        /**
         * Adds a timed call to a function in a given amount of time.
         *
         * @param callback The callback function.
         * @param callbackArgs The arguments to the function.
         * @param timeout The timeout value.
         */
        timeevent_add: function(callback, callbackArgs, timeout)
        {
            print("Skipping " + timeout + " time and calling " + callback + " directly with args = " + callbackArgs);

            var thisObj = {};
            thisObj.__proto__ = LegacyScriptPrototype;
            callback.apply(thisObj, callbackArgs);
        },

        /**
         * Returns whether combat is occurring.
         */
        combat_is_active: function() {
            return false;
        },

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

        obj_create: function(prototype, location) {
            var obj = {};
            obj.id = generateGuid();
            obj.prototype = prototype;
            obj.position = location;
            obj.map = Maps.currentMap;
            connectToPrototype(obj);

            Maps.currentMap.mobiles.push(obj);
            obj.createRenderState();

            return new CritterWrapper(obj);
        },

        /**
         * Updates the utility bar if the can-sleep-safely status has changed.
         */
        sleep_status_update: function() {
            UtilityBarUi.update();
        }
    };

    // This fixes a botched vanilla script that has a typo in it.
    GameFacade.timevent_add = GameFacade.timeevent_add;

    /*
     Copy all functions from utilities.py over to the legacy prototype.
     */
    var UtilityModule = eval('(' + readFile('scripts/legacy/utilities.js') + ')');

    var LegacyScriptPrototype = {
        SKIP_DEFAULT: true,

        RUN_DEFAULT: false,

        stat_gender: 'gender',

        gender_female: Female,

        gender_male: Male,

        OLC_NPC: 'NonPlayerCharacter',

        OLC_PC: 'PlayerCharacter',

        OLC_CONTAINER: 'Container',

        OLC_SCENERY: 'Scenery',

        OLC_CRITTERS: 'Critter',

        Q_IsFallenPaladin: 'fallen_paladin',

        Q_Is_BreakFree_Possible: 'break_free_possible',

        Q_Prone: 'is_prone',

        Q_Critter_Is_Blinded: 'is_blind',

        Q_Critter_Can_Find_Traps: 'critter_can_find_traps',

        OF_OFF: 'disabled',

        game: GameFacade,

        __proto__: UtilityModule,

        anyone: function(creatures, command, id) {
            print("Checking whether " + creatures + " have " + command + " " + id);

            var j;

            for (var i = 0; i < creatures.length; ++i) {
                var critter = creatures[i].obj;

                switch (command) {
                    case 'has_item':
                        if (critter.content) {
                            for (j = 0; j < critter.content.length; ++j) {
                                var item = critter.content[j];
                                if (item.internalId == id)
                                    return true;
                            }
                        }
                        break;
                    default:
                        print("Unknown verb for 'anyone': " + command);
                        return false;
                }
            }

            return false;
        },

        create_item_in_inventory: function(prototypeId, receiver) {

            var prototype = prototypes[prototypeId];

            if (!prototype) {
                print("Unknown prototype: " + prototype);
                return;
            }

            var item = {
                id: generateGuid(),
                prototype: prototypeId
            };
            connectToPrototype(item);

            if (!receiver.obj.content) {
                receiver.obj.content = [];
            }
            receiver.obj.content.push(item);

            print("Created item " + item.id + " with prototype " + prototypeId + " for " + receiver.obj.id);
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

    var CritterWrapper = function(obj) {
        if (!(this instanceof CritterWrapper)) {
            print("You must use new CritterWrapper instead of CritterWrapper(...)!");
        }
        if (!obj) {
            print("Constructing an object wrapper for a null-object: Very bad idea.");
        }

        // Some accesses are properties, not function calls. Set them here
        if (obj.map) {
            this.map = obj.map.id;
            this.area = obj.map.area;
        }
        this.internalId = obj.internalId;
        this.position = obj.position;

        this.obj = obj;
    };

    /**
     * Checks if the NPC wrapped by this object has met the given player character yet.
     * @param character A player character.
     */
    CritterWrapper.prototype.has_met = function(character) {
        return this.obj.hasBeenTalkedTo;
    };

    /**
     * Checks line-of-sight and invisibility/concealment, so that it is assured this critter can see the
     * other critter.
     * @param critter A critter.
     */
    CritterWrapper.prototype.can_see = function(critter) {
        return true; // TODO: Implement
    };

    /**
     * Returns the distance between this critter and another critter.
     *
     * @param critter A critter.
     */
    CritterWrapper.prototype.distance_to = function(critter) {
        // TODO: This is probably NOT in pixels but rather in world-tiles
        return distance(critter.obj.position, this.obj.position) / 28;
    };

    /**
     * Turns the NPC towards another critter.
     * @param character The character to turn to.
     */
    CritterWrapper.prototype.turn_towards = function(character) {
        // TODO: Implement
    };

    /**
     * Transfers an item from the posession of this character to the
     * posession of another character.
     */
    CritterWrapper.prototype.item_transfer_to = function(to, internalId) {
        var from = this.obj;
        to = to.obj;

        if (!from.content)
            return false;

        // TODO: This needs refactoring to account for several things
        // i.e. Updating inventory screens, taking weight updates into account,
        // scripting events, etc. etc. (or even a unified party inventory)
        for (var i = 0; i < from.content.length; ++i) {
            var item = from.content[i];
            if (item.internalId == internalId) {
                to.content.push(item);
                from.content.splice(i, 1);
                return true;
            }
        }

        return false;
    };

    /**
     * Destroys the object.
     */
    CritterWrapper.prototype.destroy = function() {
        this.obj.destroy();
    };

    /**
     * Enables a flag of this object.
     */
    CritterWrapper.prototype.object_flag_set = function(flagName) {

        switch (flagName) {
            case "disabled":
                this.obj.disable();
                break;
            default:
                print("Enabling unknown object flag on " + this.obj.id + ": " + flagName);
                break;
        }

    };

    /**
     * Disables a flag of this object.
     */
    CritterWrapper.prototype.object_flag_unset = function(flagName) {

        switch (flagName) {
            case "disabled":
                this.obj.enable();
                break;
            default:
                print("Disabling unknown object flag on " + this.obj.id + ": " + flagName);
                break;
        }

    };

    /**
     * Queries D20 state from the player.
     * @param query What to query.
     */
    CritterWrapper.prototype.d20_query = function(query) {
        switch (query) {
            case 'fallen_paladin':
                return 0; // TODO: Implement
            case 'break_free_possible':
                return 0; // TODO: Implement
            case 'is_prone':
                return 0; // TODO: Implement
            case 'is_blind':
                return 0; // TODO: Implement
            case 'critter_can_find_traps':
                return 0; // TODO: Implement
            default:
                print("Unknown d20 query.");
                return 0;
        }
    };

    /**
     * Returns a list of the character in this characters group.
     * Have to check: Is this including followers or not?
     */
    CritterWrapper.prototype.group_list = function() {
        var result = [];
        Party.getMembers().forEach(function(critter) {
            result.push(new CritterWrapper(critter));
        });
        return result;
    };

    /**
     * Returns the current value of a statistic.
     * @param stat The statistic to return.
     */
    CritterWrapper.prototype.stat_level_get = function(stat) {
        if (stat == 'gender')
            return this.obj.gender;

        print("Retrieving statistic: " + stat);
        return 0;
    };

    /**
     * Returns the leader of an NPC. This is rather odd, since every party member should be considered the leader.
     * Instead, we'll return the first player character in the party if the NPC is a follower, null otherwise.
     */
    CritterWrapper.prototype.leader_get = function() {
        if (Party.isMember(this.obj)) {
            return new CritterWrapper(Party.getPlayers()[0]);
        } else {
            return null;
        }
    };

    /**
     * This should return the current money-level for a player character in coppers (?)
     */
    CritterWrapper.prototype.money_get = function() {
        return Party.money.getTotalCopper();
    };

    /**
     * Returns whether the PC cannot accept more followers.
     */
    CritterWrapper.prototype.follower_atmax = function() {
        return false;
    };

    /**
     * Returns the unadjusted reaction value.
     */
    CritterWrapper.prototype.reaction_get = function(towards) {
        return this.obj.reaction;
    };

    /**
     * Changes the reaction value of a critter.
     */
    CritterWrapper.prototype.reaction_set = function(towards, value) {
        this.obj.reaction = value;
    };

    /**
     * Adjusts the reaction value of a critter by the given delta.
     */
    CritterWrapper.prototype.reaction_adj = function(towards, delta) {
        this.obj.reaction += delta;
    };

    /**
     * Adds a new follower to the party, if there's still room.
     * @param npc The npc to add to the party.
     */
    CritterWrapper.prototype.follower_add = function(npc) {
        if (!npc || !npc.obj) {
            print("Passed null object to follower_add.");
            return false;
        }
        return Party.addFollower(npc.obj);
    };

    /**
     * Removes a follower from the party.
     * @param npc
     */
    CritterWrapper.prototype.follower_remove = function(npc) {
        if (!npc || !npc.obj) {
            print("Passed null object to follower_remove");
            return false;
        }
        return Party.removeFollower(npc.obj);
    };

    /**
     * Changes the wealth of the party.
     * @param delta The amount of money to adjust. This may be negative or positive.
     */
    CritterWrapper.prototype.money_adj = function(delta) {
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
    CritterWrapper.prototype.skill_level_get = function(against, skill) {
        return 5;
    };

    /**
     * Check the inventory of this critter for an item with the given internal id.
     * @param internalId The internal id of the item.
     */
    CritterWrapper.prototype.item_find = function(internalId) {
        // TODO: Decide whether there should be a shared party inventory and if so, fix this accordingly.

        if (!this.obj.content)
            return null; // No inventory

        for (var i = 0; i < this.obj.content.length; ++i) {
            var item = this.obj.content[i];
            if (item.internalId == internalId)
                return item;
        }

        return null;
    };

    /**
     * This method causes NPCs to rummage through their inventory and equip all the items
     * they can (the best of their respective type).
     */
    CritterWrapper.prototype.item_wield_best_all = function() {
        // TODO: Implement
    };

    /**
     * Wraps an item for legacy scripts.
     * @param item The item to wrap.
     */
    var ItemWrapper = function(item) {
        this.item = item;
    };

    // The current dialog UI if it's open
    var conversationDialog = null;

    /**
     * Opens the dialog interface and pauses the game. This should only be called on player characters.
     *
     * @param npc The NPC that should start the conversation.
     * @param line The dialog line to start on.
     */
    CritterWrapper.prototype.begin_dialog = function(npc, line) {
        print("Trying to open dialog.");
        if (conversationDialog) {
            print("Skipping dialog, because the dialog is already open.");
            /**
             * Thanks to some scripting bugs, begin_dialog can be called multiple times in a row, causing
             * bugs if we replace dialogs...
             */
            return;
        }

        GameFacade.party = [];
        Party.getMembers().forEach(function(member) {
            GameFacade.party.push(new CritterWrapper(member));
        });

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

        var answers = [];
        for (var i = line + 1; i <= line + 50; ++i) {
            var answerLine = dialog[i];

            if (answerLine && answerLine.intelligence) {
                print("Found PC answer line " + i + " in dialog " + dialogId);

                // Check the guard
                if (!checkGuards(npc, pc, answerLine))
                    continue;

                answerLine = LegacyDialog.transformLine(dialog, answerLine, npc.obj, pc.obj);

                answers.push({
                    id: i,
                    text: answerLine.text
                });
            } else if (answerLine && !answerLine.intelligence) {
                print("Breaking out of the answer line search @ " + i);
                break; // There may be 2 PC lines, then a NPC line then more PC lines.
            }
        }

        if (answers.length == 0) {
            print("Found a dialog deadlock on line " + line + " of dialog id " + dialogId);
            answers.push({
                id: -1,
                text: '[DIALOG BUG: NOT A SINGLE ANSWER MATCHES]'
            });
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
        var filename = 'sound/speech/';
        if (dialogId < 10000)
            filename += '0';
        if (dialogId < 1000)
            filename += '0';
        if (dialogId < 100)
            filename += '0';
        if (dialogId < 10)
            filename += '0';
        filename += dialogId + '/v' + npcLine.id + '_m.mp3';

        print("Playing sound: " + filename);
        voiceOver = gameView.audioEngine.playSoundOnce(filename, SoundCategory_Effect);

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

            if (line == -1)
                return; // This was the "escape line" for buggy dialogs

            // Interestingly enough, B: always causes a barter, even if "pc.barter" is not the action
            if (dialog[line].text.substring(0, 2) == 'B:') {
                MerchantUi.show(npc.obj, pc.obj);
                return;
            }

            var action = dialog[line].action;

            if (action) {
                script.performAction(npc, pc, action);
            }

            // Now we have talked to the NPC
            npc.obj.hasBeenTalkedTo = true;

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

        return script.san_dialog(new CritterWrapper(attachedTo), new CritterWrapper(getTriggerer()));
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

        return script.san_join(new CritterWrapper(attachedTo), new CritterWrapper(getTriggerer()));
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

        return script.san_disband(new CritterWrapper(attachedTo), new CritterWrapper(getTriggerer()));
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

        return script.san_use(new CritterWrapper(attachedTo), new CritterWrapper(getTriggerer()));
    };

    /**
     * Triggers the first heartbeat event for a legacy script. This event is triggered once
     * when a map is loaded.
     *
     * @param attachedScript The legacy script. This is a JavaScript object with at least a 'script' property giving
     *                 the id of the legacy script.
     * @param attachedTo The object the script is attached to.
     */
    LegacyScripts.OnFirstHeartbeat = function(attachedScript, attachedTo) {
        var script = getScript(attachedScript.script);

        if (!script) {
            print("Unknown legacy script: " + attachedScript.script);
            return false;
        }

        return script.san_first_heartbeat(new CritterWrapper(attachedTo), null);
    };

    /**
     * Triggers the heartbeat event for a legacy script. This event is triggered in regular intervals.
     *
     * @param attachedScript The legacy script. This is a JavaScript object with at least a 'script' property giving
     *                 the id of the legacy script.
     * @param attachedTo The object the script is attached to.
     */
    LegacyScripts.OnHeartbeat = function(attachedScript, attachedTo) {
        var script = getScript(attachedScript.script);

        if (!script) {
            print("Unknown legacy script: " + attachedScript.script);
            return false;
        }

        return script.san_heartbeat(new CritterWrapper(attachedTo), null);
    };

    /**
     * Triggers the enter combat event for a legacy script.
     *
     * @param attachedScript The legacy script. This is a JavaScript object with at least a 'script' property giving
     *                 the id of the legacy script.
     * @param attachedTo The object the script is attached to.
     */
    LegacyScripts.OnEnterCombat = function(attachedScript, attachedTo) {
        var script = getScript(attachedScript.script);

        if (!script) {
            print("Unknown legacy script: " + attachedScript.script);
            return false;
        }

        return script.san_enter_combat(new CritterWrapper(attachedTo), null);
    };

    /**
     * Triggered when the NPC enters a new map.
     *
     * @param attachedScript The legacy script. This is a JavaScript object with at least a 'script' property giving
     *                 the id of the legacy script.
     * @param attachedTo The object the script is attached to.
     */
    LegacyScripts.OnNewMap = function(attachedScript, attachedTo) {
        var script = getScript(attachedScript.script);

        if (!script) {
            print("Unknown legacy script: " + attachedScript.script);
            return false;
        }

        return script.san_new_map(new CritterWrapper(attachedTo), null);
    };

})();
