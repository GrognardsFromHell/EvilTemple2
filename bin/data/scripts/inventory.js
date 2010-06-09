
var inventoryIcons = {};

function loadInventoryIcons() {
	inventoryIcons = eval('(' + readFile('inventoryIcons.js') + ')');
}

function getInventoryIconPath(id) {
	return 'art/interface/inventory/' + inventoryIcons[id];
}
