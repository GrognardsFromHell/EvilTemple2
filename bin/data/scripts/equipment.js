
var equipment = {
};

var modelToTypeMapping = {
        "meshes/pcs/pc_human_male/pc_human_male.model": 'human-male',
        "meshes/npcs/elmo/npc_elmo.model": 'human-male',
        "meshes/npcs/brother_smith/npc_brother_smith.model": 'human-male',
        "meshes/pcs/pc_human_female/pc_human_female.model": 'human-female',
        "meshes/pcs/pc_halforc_male/pc_halforc_male.model": 'halforc-male',
        "meshes/pcs/pc_halforc_female/pc_halforc_female.model": 'halforc-female'
};

function loadEquipment() {
        equipment = eval('(' + readFile('equipment.js') + ')');
}

function updateEquipment(obj, modelInstance) {
    var modelId = obj.model.replace(/\\/g, '/').toLowerCase();

    var type = modelToTypeMapping[modelId];

    if (type !== undefined) {
            var materials = {
                    'HEAD': '',
                    'GLOVES': '',
                    'CHEST': '',
                    'BOOTS': ''
            };

            function processEquipment(id) {
                    var record = equipment[id][type];

                    if (record === undefined) {
                            print('Unknown wear mesh id: ' + id);
                            return;
                    }

                    if (record.materials !== undefined) {
                            for (var k in record.materials) {
                                    materials[k] = record.materials[k];
                            }
                    }

                    if (record.meshes !== undefined) {
                            for (var i = 0; i < record.meshes.length; ++i) {
                                    modelInstance.addMesh(models.load(record.meshes[i]));
                            }
                    }
            }

            // Naked "equipment" (should be merged)
            processEquipment(0);
            processEquipment(2);
            processEquipment(5);
            processEquipment(8);

            if (obj.content !== undefined) {
                    // Check all the equipment the object has
                    for (var i = 0; i < obj.content.length; ++i) {
                            var childObj = obj.content[i];
                            connectToPrototype(childObj);

                            if (childObj.wearMeshId > 0) {
                                    processEquipment(childObj.wearMeshId);
                            }
                    }
            }

            for (var placeholder in materials) {
                    modelInstance.overrideMaterial(placeholder, gameView.materials.load(materials[placeholder]));
            }

            if (obj.hairType !== undefined && obj.hairColor !== undefined) {
                    var gender = 'male'; // obj.gender;
                    var genderShort = (gender == 'male') ? 'm' : 'f';

                    var style = obj.hairType;
                    var color = obj.hairColor;

                    var filename = 'meshes/hair/' + gender + '/s' + style + '/hu_' + genderShort + '_s' + style + '_c' + color + '_small.model';
                    modelInstance.addMesh(models.load(filename));
            }

            // modelInstance.addMesh(models.load('meshes/armor/fullplate_addmesh.model'));
    }

    // also add explicitly requested addmeshes
    if (obj.addMeshes !== undefined) {
            for (var i = 0; i < obj.addMeshes.length; ++i) {
                    modelInstance.addMesh(models.load(obj.addMeshes[i]));
            }
    }
}
