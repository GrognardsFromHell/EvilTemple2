
function selectFeat(id) {
    var newSelectedFeats = [id];
    for (var i = 0; i < selectedFeats.length; ++i) {
        newSelectedFeats.push(selectedFeats[i]);
    }
    console.log("New selected feats: " + newSelectedFeats);

    selectedFeats = newSelectedFeats;
}

function deselectFeat(id) {
    var newSelectedFeats = [];
    for (var i = 0; i < selectedFeats.length; ++i) {
        newSelectedFeats.push(selectedFeats[i]);
    }
    newSelectedFeats.splice(newSelectedFeats.indexOf(id), 1);
    selectedFeats = newSelectedFeats;
}
