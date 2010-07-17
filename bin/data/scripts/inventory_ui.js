function showInventory(obj) {
    var inventoryDialog = gameView.addGuiItem("interface/Inventory.qml");

    var objects = [];
    if (obj.content !== undefined) {
        for (var i = 0; i < obj.content.length; ++i) {
            var item = obj.content[i];
            if (item.inventoryId === undefined || item.descriptionId === undefined) {
                print("Object lacks inventory id or description id.");
                continue;
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
            objects.push({
                guid: item.id,
                iconPath: getInventoryIconPath(item.inventoryId),
                description: translations.get('mes/description/' + item.descriptionId),
                location: item.itemInventoryLocation,
                quantity: quantity,
                magical: magical,
                weight: item.weight
            });
        }
    }

    function findItem(container, guid) {
        if (!container.content)
            return null;

        for (var i = 0; i < container.content.length; ++i) {
            var item = container.content[i];
            if (item.id == guid) {
                return item;
            }
        }

        return null;
    }

    inventoryDialog.itemRightClicked.connect(function (guid) {
        var item = findItem(obj, guid);
        if (item) {
            showMobileInfo(item, null);
        }
    });

    inventoryDialog.itemDoubleClicked.connect(function (guid) {
        var item = findItem(obj, guid);
        if (item) {
            obj.content.splice(obj.content.indexOf(item), 1); // Remove from container

            inventoryDialog.removeItem(guid);

            var player = Party.getPlayers()[0];
            if (!player.content)
                player.content = [];
            player.content.push(item);
        }
    });

    var money = new Money(obj.money);
    print("Object money: " + obj.money);

    inventoryDialog.money = money.getTotalCopper();
    inventoryDialog.items = { objects: objects };

    inventoryDialog.closeClicked.connect(function() {
        inventoryDialog.deleteLater();
    });
}
