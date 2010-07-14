/**
 * Manages legacy dialog files.
 */
var LegacyDialog = {};

(function() {

    var dialogMapping;

    var dialogCache = {};

    function loadDialog(id) {
        var filename = dialogMapping[id];

        if (filename === undefined) {
            return undefined;
        }

        // TODO: Set a prototype on the resulting object?

        try {
            return eval('(' + readFile(filename) + ')');
        } catch(err) {
            print("Unable to read legacy dialog file " + filename);
            throw err;
        }
    }

    /**
     * Retrieves a legacy dialog object.
     * @param id The id of the legacy dialog.
     * @returns A legacy dialog object or undefined if the id is unknown.
     */
    function getDialog(id) {
        var obj = dialogCache[id];

        if (obj === undefined) {
            obj = loadDialog(id);
            dialogCache[id] = obj;
        }

        return obj;
    }

    /**
     * Retrieves the actual text-line that should be shown in a conversation.
     *
     * @param line The line object whose text should be retrieved
     * @param npc The NPC that is talking.
     * @param pc The PC that is talking.
     * @returns A new line that may have transformed text and other properties.
     */
    LegacyDialog.transformLine = function(dialog, line, npc, pc) {

        // Make a copy
        var result = {
            text: line.text,
            femaleText: line.femaleText,
            nextId: line.nextId,
            guard: line.guard
        };

        // Use the gender-specific text if applicable
        if (pc.gender == 'female' && line.femaleText) {
            text = line.femaleText;
        }

        var translationBase = 'mes/gd_pc2m/';
        if (npc.gender == 'female')
            translationBase = 'mes/gd_pc2f/';

        var text = result.text;
        if (pc.gender == 'female' && result.femaleText)
            text = result.femaleText;

        // Check for various random responses
        if (text == 'E:') {
            // PC2NPC Exit message
            result.text = translations.get(translationBase + randomRange(400, 428));
        } else if (text == 'A:') {
            // PC2NPC Appreciation message
        } else if (text == 'B:') {
            // PC2NPC Barter message (Note: can always be followed by some more text, so this wont match)
        } else if (text == 'K:') {
            // PC2NPC Can i ask some more questions
            result.text = translations.get(translationBase + randomRange(1500, 1514));
        } else if (text == 'F:') {
            // PC2NPC Forget it message
            result.text = translations.get(translationBase + randomRange(800, 806));
        } else if (text == 'S:') {
            // PC2NPC Sorry message.
            result.text = translations.get(translationBase + randomRange(200, 225));
        } else if (text == 'N:') {
            // PC2NPC No message.
            result.text = translations.get(translationBase + randomRange(100, 118));
        } else if (text == 'Y:') {
            // PC2NPC Yes message.
            result.text = translations.get(translationBase + randomRange(1, 20));
        } else if (text == 'R:') {
            // PC2NPC Rumors... This actually causes the dialog to intervene and go for a separate rumor-dialog.
        } else if (text == 'G:') {
            /*
             Special cases currently not handled:

             gd_cls_m2m.mes:
             - Greeting message if PC is undressed, dialog file 10015, mes: 16000
             - Greeting message if PC is in barbarian armor, dialog file 10016, mes:  17000
             - Fear of associates? (PC has summoned monsters?) - dialog: 10017 mes: 18000
             - PC is polymorphed - dialog: 10019, mes: 20000
             - PC has mirror-images on him - dialog: 10020, mes: 21000
             - PC is invisible - dialog: 10021, mes: 22000
             ----
             gd_rce_m2m.mes:
             - PC has shrink spell on him, dialog file: (probably 11005), mes: 6000
             ----

             Reputation could also be factored into the greeting (game_rp_npc_*.mes)

             */

            // NPC2PC Greeting message. This can come either from a generic file, or from the NPCs actual dialog
            var reaction = npc.getReaction();

            /**
             * This should probably not be modified here.
             */
            var firstConversation = false;
            if (!npc.hasBeenTalkedTo) {
                npc.hasBeenTalkedTo = true;
                firstConversation = true;
            }

            function replaceLine(newLineId) {
                if (!dialog[newLineId])
                    return false;
                var newLine = dialog[newLineId];
                result.text = newLine.text;
                result.femaleText = newLine.text;
                result.guard = newLineId;
                return true;
            }

            switch (reaction) {
                case Reaction.Great:
                    if (firstConversation) {
                        if (!replaceLine(10008))
                            result.text = translations.get('mes/gd_cls_m2m/' + randomRange(8000, 8009));
                    } else {
                        if (!replaceLine(10011))
                            result.text = translations.get('mes/gd_cls_m2m/' + randomRange(11000, 11009));
                    }
                    break;
                case Reaction.Good:
                    if (firstConversation) {
                        if (!replaceLine(10008))
                            result.text = translations.get('mes/gd_cls_m2m/' + randomRange(9000, 9036));
                    } else {
                        if (!replaceLine(10011))
                            result.text = translations.get('mes/gd_cls_m2m/' + randomRange(12000, 12020));
                    }
                    break;
                case Reaction.Neutral:
                    if (firstConversation) {
                        if (!replaceLine(10009))
                            result.text = translations.get('mes/gd_cls_m2m/' + randomRange(10000, 10030));
                    } else {
                        if (!replaceLine(10012))
                            result.text = translations.get('mes/gd_cls_m2m/' + randomRange(13000, 13016));
                    }
                    break;
                case Reaction.Bad:
                    if (firstConversation) {
                        if (!replaceLine(11001))
                            result.text = translations.get('mes/gd_rce_m2m/' + randomRange(2000, 2014));
                    } else {
                        if (!replaceLine(11003))
                            result.text = translations.get('mes/gd_rce_m2m/' + randomRange(4000, 4017));
                    }
                    break;
                case Reaction.Worst:
                    if (firstConversation) {
                        if (!replaceLine(11002))
                            result.text = translations.get('mes/gd_rce_m2m/' + randomRange(3000, 3009));
                    } else {
                        if (!replaceLine(11004))
                            result.text = translations.get('mes/gd_rce_m2m/' + randomRange(5000, 5019));
                    }
                    break;
            }
        }

        // Replace the PC's name
        result.text = result.text.replace('@pcname@', pc.getName());
        if (result.femaleText)
            result.femaleText = result.femaleText.replace('@pcname@', pc.getName());

        return result;
    };

    /**
     * Loads the legacy dialog model.
     */
    LegacyDialog.load = function() {
        dialogMapping = eval('(' + readFile('dialogs.js') + ')');

        // This counting method may be inaccurate, but at least it gives us some ballpark estimate
        var count = 0;
        for (var k in dialogMapping)
            count++;
        print("Loaded " + count + " legacy dialogs.");
    };

    /**
     * Retrieves a legacy dialog object by id.
     * @param id The id of the legacy dialog.
     */
    LegacyDialog.get = function(id) {
        return getDialog(id);
    };
})();
