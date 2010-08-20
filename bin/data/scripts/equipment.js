var Equipment = {};

var equipment = {
};

(function() {

    /**
     * Returns a map of the override materials and addmeshes required to render an object's
     * equipment.
     *
     * @param obj The object to return the override materials for.
     * @returns An object with two properties:
     * - materials will contain a map from material placeholders to material filenames.
     * - meshes will contain an array of model filenames that need to be added to the model.
     */
    Equipment.getRenderEquipment = function(obj) {
        var result = {
            materials: {},
            meshes: []
        };

        if (!obj.race || !obj.gender)
            return result;

        var type = obj.race + '-' + obj.gender;

        function processEquipment(id) {
            var record = equipment[id][type];

            if (!record) {
                print('Unknown wear mesh id: ' + id + ' or type: ' + type);
                return;
            }

            if (record.materials) {
                for (var k in record.materials) {
                    result.materials[k] = record.materials[k];
                }
            }

            if (record.meshes) {
                record.meshes.forEach(function(filename) {
                    result.meshes.push(filename);
                });
            }
        }

        // Naked "equipment" (should be merged)
        processEquipment(0);
        processEquipment(2);
        processEquipment(5);
        processEquipment(8);

        if (obj.content) {
            // Check all the equipment the object has
            obj.content.forEach(function (childObj) {
                connectToPrototype(childObj);

                if (childObj.wearMeshId > 0) {
                    processEquipment(childObj.wearMeshId);
                }
            });
        }

        return result;
    };
})();

function loadEquipment() {
    equipment = eval('(' + readFile('equipment.js') + ')');
}

function updateEquipment(obj, modelInstance) {
    var type = null;

    if (obj.race && obj.gender) {
        type = obj.race + '-' + obj.gender;
    }

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
    }

    // Add explicitly requested addmeshes
    if (obj.addMeshes !== undefined) {
        obj.addMeshes.forEach(function(filename) {
            modelInstance.addMesh(models.load(filename));
        });
    }
}
