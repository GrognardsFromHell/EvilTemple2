    
function openCharacterSheet(critter) {

    var dialog = gameView.addGuiItem("interface/CharacterSheet.qml");
    dialog.closeClicked.connect(function() {
        dialog.deleteLater();
    });    
        
    dialog.name = translations.get('mes/description/' + critter.descriptionId);
    
    if (critter.portrait !== undefined)
        dialog.portrait = getPortrait(critter.portrait, Portrait_Medium);

    dialog.strength = critter.strength;
    dialog.dexterity = critter.dexterity;
    dialog.constitution = critter.constitution;
    dialog.intelligence = critter.intelligence;
    dialog.wisdom = critter.wisdom;
    dialog.charisma = critter.charisma;
    
    var levels = '';
    if (critter.classLevels !== undefined) {
        for (var i = 0; i < critter.classLevels.length; ++i) {
            if (levels != '')
                levels += ', ';
                
            var classLevel = critter.classLevels[i];
            levels += classLevel['class'] + ': ' + classLevel.count;
        }
    }
    dialog.levels = levels;
    
    var skills = [];
    if (critter.skills !== undefined) {
        for (var k in critter.skills) {
            skills.push({
                'name': k,
                'ranks': critter.skills[k]
            });
        }
    }
    dialog.skills = skills;

}
