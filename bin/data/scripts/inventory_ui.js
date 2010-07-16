
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

    inventoryDialog.itemClicked.connect(function (guid) {
        for (var i = 0; i < obj.content.length; ++i) {
            var item = obj.content[i];
            if (item.id == guid) {
                showMobileInfo(item, null);
                return;
            }
        }
    });

    var money = new Money(obj.money);
    print("Object money: " + obj.money);

    inventoryDialog.money = money.getTotalCopper();
    inventoryDialog.items = objects;
    
    inventoryDialog.closeClicked.connect(function() {
        inventoryDialog.deleteLater();
    });
}
