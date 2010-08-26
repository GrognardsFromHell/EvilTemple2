
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

    mobileInfoDialog.setPortrait(Portraits.getPortrait(obj.portrait, Portrait.Medium));
    mobileInfoDialog.title = 'Property View';
    mobileInfoDialog.setItems(items);
    mobileInfoDialog.closeClicked.connect(function() {
        mobileInfoDialog.deleteLater();
    });
    mobileInfoDialog.deleteClicked.connect(function() {
        mobileInfoDialog.deleteLater();
        obj.destroy();
    });

    if (modelInstance) {
        mobileInfoDialog.openAnimations.connect(function() {
            openAnimations(modelInstance);
        });
    }

    mobileInfoDialog.hasInventory = (obj.content !== undefined && obj.content.length > 0);
    mobileInfoDialog.openInventory.connect(function() {
        if (obj.content !== undefined && obj.content.length > 0) {
            showInventory(obj);
        } else {
            print("Object has no contents. Skipping.");
        }
    });
}
