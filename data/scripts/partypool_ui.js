var PartyPoolUi = {};

(function() {

    function mapCharacter(vaultChar) {
        // Evaluate the character from the vault to get some properties.
        var character = eval('(' + vaultChar.script + ')');
        print("Loaded character " + character.id + " " + character.name + " from the vault.");

        var portrait = Portraits.getImage(character.portrait, Portrait.Medium);
        var gender = (character.gender == Gender.Male) ? qsTr('Male') : qsTr('Female');
        var race = Races.getById(character.race).name;
        var compatible = CompatibleAlignments[character.alignment].indexOf(Party.alignment) != -1;

        var classLevelText = '';
        character.classLevels.forEach(function (classLevel) {
            classLevelText += Classes.getById(classLevel.classId).name;
            classLevelText += ' ' + classLevel.count;
        });

        return {
            id: character.id,
            name: character.name,
            portrait: portrait,
            gender: gender,
            race: race,
            alignment: AlignmentNames[character.alignment],
            classes: classLevelText,
            filename: vaultChar.filename,
            compatible: compatible
        };
    }

    function update(dialog) {
        // Now "re-set" the characters list
        var selectedParty = dialog.getPartySelection();
        dialog.characters = charactervault.list().map(mapCharacter);
        dialog.setPartySelection(selectedParty);
    }

    /**
     * Shows the party creation interface.
     *
     * @param startGameCallback Called when the player presses the start game button.
     * @param cancelCallback Called when the player cancels the party creation.
     */
    PartyPoolUi.showCreateParty = function(startGameCallback, cancelCallback) {

        var dialog = gameView.showView('interface/PartyVault.qml');

        dialog.characters = charactervault.list().map(mapCharacter);

        dialog.beginAdventuringClicked.connect(function() {

            dialog.deleteLater();

            // Get the selected party, create corresponding characters / add them to the party
            // then start the game
            var selectedIds = dialog.getPartySelection();

            var characters = charactervault.list().map(function (vaultChar) {
                return eval('(' + vaultChar.script + ')');
            });

            selectedIds.forEach(function (id) {
                var found = false;

                // Find the corresponding char in characters
                for (var i = 0; i < characters.length; ++i) {
                    if (characters[i].id != id)
                        continue;

                    var instance = characters[i]; // The instances are *fresh* due to being eval'd above
                    Prototypes.reconnect(instance);
                    Party.addMember(instance);
                    found = true;
                    break;
                }

                if (!found) {
                    print("WARNING: Didn't find character with id " + id + " although it should be in the party.");
                }
            });

            startGameCallback();
        });

        dialog.createCharacterClicked.connect(function() {
            dialog.visible = false;

            // Show character creation-ui.
            var callback = function() {
                dialog.visible = true;
                update(dialog);
            };

            CreateCharacterUi.show(callback, callback);
        });

        dialog.deleteCharacterClicked.connect(function (filename) {
            var okayCallback = function() {
                print("Deleting character: " + filename);
                charactervault.remove(filename);

                update(dialog);
            };

            var cancelCallback = function() {
            }; // No-op function

            var message = "Do you really want to delete the character " + filename + "?";
            ModalDialogUi.show("Delete Character", message, okayCallback, cancelCallback);
        });

        dialog.viewCharacterClicked.connect(function(id) {
            print("Viewing character " + id);
        });

        dialog.closeClicked.connect(function() {
            dialog.deleteLater();
            cancelCallback();
        });

    };

})();
