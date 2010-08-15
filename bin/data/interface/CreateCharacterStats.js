
var characterStatItems = {};

function disableStatsDropHighlight() {
    for (var k in characterStatItems) {
        characterStatItems[k].highlight = false;
    }
}

function highlightStatsDrop(x, y) {
    var stat = findStatsDrop(x, y);

    for (var k in characterStatItems) {
        characterStatItems[k].highlight = (k == stat);
    }
}

function registerCharStatItem(key, root) {
    console.log("Registering " + key);
    characterStatItems[key] = root;
}

/**
    Returns the stat's name closest to the X,Y coordinate (relative to the stats group box)
*/
function findStatsDrop(x, y) {
    // Check in the vicinity of the box being dragged, whether there is a drop-enabled box

    var canidates = [];

    for (var k in characterStatItems) {
        var dropZone = characterStatItems[k];

        var pos = root.mapFromItem(dropZone, dropZone.width / 2, dropZone.height / 2);

        var dist = Utilities.distance(pos.x,
                                      pos.y,
                                      x,
                                      y);

        if (dist <= 35) {
            canidates.push([dist, k]);
        }
    };

    if (canidates.length == 0)
        return null;

    canidates.sort(function (a, b) {
        return a[0] - b[0];
    });

    return canidates[0][1];
}

function swapStats(statSource1, statSource2) {
    print("Swapping " + statSource1 + " on " + statSource2);

    var tmp = characterStatItems[statSource1].statValue;
    characterStatItems[statSource1].statValue = characterStatItems[statSource2].statValue;
    characterStatItems[statSource2].statValue = tmp;

    updateStatsLabels();
}

function updateStatsLabels() {
    strength = parseInt(characterStatItems['str'].statValue);
    dexterity = parseInt(characterStatItems['dex'].statValue);
    constitution = parseInt(characterStatItems['con'].statValue);
    intelligence = parseInt(characterStatItems['int'].statValue);
    wisdom = parseInt(characterStatItems['wis'].statValue);
    charisma = parseInt(characterStatItems['cha'].statValue);

    complete = (strength > 0 && dexterity > 0 && constitution > 0
        && intelligence > 0 && wisdom > 0 && charisma > 0);
}

function rollD6() {
    return 1 + Math.floor(Math.random() * 6); // This is 0-5;
}

function rollStats(reroll) {
    // Reset all to zero
    for (var k in characterStatItems)
        characterStatItems[k].statValue = '0';

    // Roll 6 values
    var valid = false;
    while (!valid) {
        var totalModifier = 0;
        for (var i = 1; i <= 6; ++i) {
            var values = [rollD6(), rollD6(), rollD6(), rollD6()];
            values.sort();
            console.debug("Discarding dice roll " + values[0] + ". Using: " + values.slice(1));
            var value = values[1] + values[2] + values[3];
            // TODO: Modificator
            characterStatItems['roll' + i].statValue = value;
        }
        valid = true;
    }

    updateStatsLabels();

    if (reroll)
        rolls++;
}
