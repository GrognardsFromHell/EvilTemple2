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

        var mobile = Maps.currentMap.findMobileById(guid);
        showMobileInfo(mobile, null);
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
     * Shows the merchant UI.
     *
     * @param merchant The merchant.
     * @param player The player who initiated the trade.
     */
    MerchantUi.show = function(merchant, player) {
        this.close();

        var inventory = merchant.content;

        // If the NPC has a substitute inventory id, look for it.
        if (merchant.substituteInventoryId) {
            var container = merchant.map.findMobileById(merchant.substituteInventoryId);

            if (!container) {
                print("Unable to find substitute inventory: " + merchant.substituteInventoryId);
                return;
            }

            inventory = container.content;
        }

        currentMerchant = merchant;
        currentPlayer = player;

        merchantUi = gameView.addGuiItem("interface/Merchant.qml");
        merchantUi.closeClicked.connect(MerchantUi, MerchantUi.close);
        merchantUi.buyItem.connect(this, buyItem);

        merchantUi.money = Party.money.getTotalCopper();
        merchantUi.merchantItems = buildModelFromInventory(inventory);
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

})();
