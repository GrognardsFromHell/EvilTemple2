
var portraits = {};

var Portrait_Large = 0;
var Portrait_Medium = 1;
var Portrait_Small = 2;

function loadPortraits() {
	print("Loading portraits...");
	portraits = eval('(' + readFile('portraits.js') + ')');	
}

function getPortrait(id, size) {
	if (size === undefined) 
		size = Portrait_Medium;
		
	var record = portraits[id];
	
	if (record === undefined) {
		record = portraits['0'];
	}
	
	if (record === undefined) {	
		return null;
	}
	
	return 'art/interface/portraits/' + record[size];
}
