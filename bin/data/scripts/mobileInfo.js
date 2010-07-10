
function addObjProps(obj, items)
{
    var proto = obj.__proto__;

    for (var k in obj) {
        if (k == 'selectionCircle')
            continue;

        // Content is handled by inventory
        if (k == 'content')
            continue;

        var value = obj[k];

        if (proto != null && proto[k] === value)
            continue;

        // Skip functions
        if (value instanceof Function)
            continue;

        if (value instanceof Array) {
            var newValue = '[';
            for (var i = 0; i < value.length; ++i) {
                var subvalue = value[i];

                if (typeof(subvalue) == 'object')
                    subvalue = objectToString(subvalue);

                if (i != 0)
                    newValue += ', ' + subvalue;
                else
                    newValue += subvalue;
            }
            value = newValue + ']';
        } else if (typeof(value) == 'object') {
            value = objectToString(value);
        }
        if (k == 'descriptionId') {
            value = translations.get('mes/description/' + value) + ' (' + value + ')';
        } else if (k == 'unknownDescriptionId') {
            value = translations.get('mes/description/' + value) + ' (' + value + ')';
        }

        items.push([k, value]);
    }

    if (proto !== null) {
        items.push(['--------', '-------------']);
        addObjProps(proto, items);
    }
}

function showMobileInfo(obj, modelInstance)
{
    var mobileInfoDialog = gameView.addGuiItem("interface/MobileInfo.qml");
    var items = [];
    addObjProps(obj, items);

    mobileInfoDialog.setPortrait(getPortrait(obj.portrait, Portrait_Medium));
    mobileInfoDialog.title = 'Property View';
    mobileInfoDialog.setItems(items);
    mobileInfoDialog.closeClicked.connect(function() {
        mobileInfoDialog.deleteLater();
    });

    mobileInfoDialog.openAnimations.connect(function() {
        openAnimations(modelInstance);
    });

    mobileInfoDialog.hasInventory = (obj.content !== undefined && obj.content.length > 0);
    mobileInfoDialog.openInventory.connect(function() {
        if (obj.content !== undefined && obj.content.length > 0) {
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
                    for (var j= 0; j < item.itemFlags.length; ++j) {
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
        } else {
            print("Object has no contents. Skipping.");
        }
    });
}
