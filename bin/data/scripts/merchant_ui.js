var MerchantUi = {
};

(function() {

    var merchantUi = null; // The currently opened merchant UI.
    var currentMerchant = null;
    var currentPlayer = null;

    /**
     * Issued when the user requests to buy an item.
     * @param guid The GUID of the item.
     */
    function buyItem(guid) {
        print("Trying to buy item " + guid);

        var inventory = getInventory(currentMerchant);

        for (var i = 0; i < inventory.length; ++i) {
            var item = inventory[i];
            if (item.id == guid) {
                // Pay for it and only if that succeeds, move it
                if (Party.money.getTotalCopper() < item.worth) {
                    print("Player doesn't have enough money.");
                    return;
                }
                Party.money.addCopper(- item.worth);

                inventory.splice(i, 1); // Remove from container

                if (!currentPlayer.content)
                    currentPlayer.content = [];

                currentPlayer.content.push(item);

                // Refresh the UI
                MerchantUi.refresh();

                return;
            }
        }

        print("Unknown item guid: " + guid);
    }

    /**
     * Builds the model from an array of items.
     * @param inventory
     */
    function buildModelFromInventory(inventory, worthAdjustment) {
        if (!worthAdjustment)
            worthAdjustment = 1;

        if (!inventory)
            return [];

        var result = [];

        inventory.forEach(function (item) {
            if (item.inventoryId === undefined || item.descriptionId === undefined) {
                print("Object lacks inventory id or description id.");
                return;
            }

            var quantity = item.quantity;
            if (!quantity)
                quantity = 1;

            var magical = false;
            if (item.itemFlags !== undefined) {
                for (var j = 0; j < item.itemFlags.length; ++j) {
                    if (item.itemFlags[j] == 'IsMagical') {
                        magical = true;
                        break;
                    }
                }
            }
            result.push({
                guid: item.id,
                iconPath: getInventoryIconPath(item.inventoryId),
                description: translations.get('mes/description/' + item.descriptionId),
                location: item.itemInventoryLocation,
                quantity: quantity,
                magical: magical,
                weight: item.weight,
                worth: worthAdjustment * item.worth
            });
        });

        return result;
    }

    /**
     * Gets the merchant's stock inventory.
     * @param merchant The merchant.
     */
    function getInventory(merchant) {
        var inventory = merchant.content;

        // If the NPC has a substitute inventory id, look for it.
        if (merchant.substituteInventoryId) {
            var container = merchant.map.findMobileById(merchant.substituteInventoryId);

            if (!container) {
                print("Unable to find substitute inventory: " + merchant.substituteInventoryId);
                return null;
            }

            inventory = container.content;
        }

        return inventory;
    }

    /**
     * Shows the merchant UI.
     *
     * @param merchant The merchant.
     * @param player The player who initiated the trade.
     */
    MerchantUi.show = function(merchant, player) {
        this.close();

        currentMerchant = merchant;
        currentPlayer = player;

        /*
         Create and initialize the merchant UI
         */
        merchantUi = gameView.addGuiItem("interface/Merchant.qml");
        merchantUi.closeClicked.connect(MerchantUi, MerchantUi.close);
        merchantUi.buyItem.connect(this, buyItem);

        merchantUi.merchantName = merchant.getName();
        merchantUi.merchantPortrait = getPortrait(merchant.portrait, Portrait_Medium);
        merchantUi.playerName = player.getName();
        merchantUi.playerPortrait = getPortrait(player.portrait, Portrait_Medium);

        this.refresh();
    };

    /**
     * Closes the merchant ui.
     */
    MerchantUi.close = function() {
        if (merchantUi)
            merchantUi.deleteLater();

        merchantUi = null;
        currentMerchant = null;
        currentPlayer = null;
    };

    /**
     * Refreshes the data displayed by the current merchant ui.
     *
     * This is used after a transaction and on the initial show of the UI.
     */
    MerchantUi.refresh = function() {
        if (!currentMerchant)
            return;

        var inventory = getInventory(currentMerchant);
        if (!inventory)
            inventory = [];
        merchantUi.merchantItems = buildModelFromInventory(inventory, 1.0);

        var playerInventory = currentPlayer.content;
        if (!playerInventory)
            playerInventory = [];
        merchantUi.playerItems = buildModelFromInventory(playerInventory, 0.5);

        merchantUi.money = Party.money.getTotalCopper();
    }

})();
