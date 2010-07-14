
function showInventory(obj) {
    var inventoryDialog = gameView.addGuiItem("interface/Inventory.qml");

    var gold = 0;
    var silver = 0;
    var copper = 0;
    var platinum = 0;

    var objects = [];
    for (var i = 0; i < obj.content.length; ++i) {
        var item = obj.content[i];
        if (item.inventoryId === undefined || item.descriptionId === undefined) {
            print("Object lacks inventory id or description id.");
            continue;
        }

        var quantity = 1;
        if (item.moneyQuantity !== undefined)
            quantity = item.moneyQuantity;
        if (item.ammoQuantity !== undefined)
            quantity = item.ammoQuantity;

        if (item.type == 'Money') {
            // This should be fixed
            switch (item.prototype) {
                case 7000:
                    copper += quantity;
                    break;
                case 7001:
                    silver += quantity;
                    break;
                case 7002:
                    gold += quantity;
                    break;
                case 7003:
                    platinum += quantity;
                    break;
                default:
                    print("Unknown money prototype: " + item.prototype);
                    break;
            }
            continue; // Don't show money in the inventory
        }

        var magical = false;
        if (item.itemFlags !== undefined) {
            for (var j = 0; j < item.itemFlags.length; ++j) {
                if (item.itemFlags[j] == 'IsMagical') {
                    magical = true;
                    break;
                }
            }
        }

        objects.push({
            iconPath: getInventoryIconPath(item.inventoryId),
            description: translations.get('mes/description/' + item.descriptionId),
            location: item.itemInventoryLocation,
            quantity: quantity,
            magical: magical
        });
    }

    inventoryDialog.setInventory({
        gold: gold,
        silver: silver,
        platinum: platinum,
        copper: copper,
        items: objects
    });
    
    inventoryDialog.closeClicked.connect(function() {
        inventoryDialog.deleteLater();
    });
}
