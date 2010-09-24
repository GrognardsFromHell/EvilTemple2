var Equipment = {};

(function() {

    var equipment = {
    };

    /**
     * Registers new equipment definition records.
     *
     * @param entries An object that contains one key-value mapping for each equipment record.
     */
    Equipment.register = function(entries) {

        var registered = 0;

        for (var k in entries) {
            if (entries.hasOwnProperty(k)) {
                equipment[k] = entries[k];
                registered++;
            }
        }

        print("Registered " + registered + " equipment records.");

    };

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
            if (!equipment[id] || !equipment[id][type]) {
                print('Unknown equipment id: ' + id + ' or type: ' + type);
                return;
            }

            var record = equipment[id][type];

            if (record.materials) {
                for (var k in record.materials)
                    result.materials[k] = record.materials[k];
            }

            if (record.meshes) {
                record.meshes.forEach(function(filename) {
                    result.meshes.push(filename);
                });
            }
        }

        // Add all naked "equipment" to the result.
        processEquipment("naked");

        // Add hair next
        if (obj.hairType) {
            processEquipment("hair-" + obj.hairType);
            // Hair-color is "added" below in addRenderEquipment
        }

        if (obj.content) {
            // Check all the equipment the object has
            obj.content.forEach(function (childObj) {
                connectToPrototype(childObj);

                if (childObj.equipmentId) {
                    processEquipment(childObj.equipmentId);
                }
            });
        }

        // Add explicitly requested addmeshes
        if (obj.addMeshes) {
            obj.addMeshes.forEach(function(filename) {
                result.meshes.push(filename);
            });
        }

        return result;
    };

    /**
     * Adds all the necessary equipment and material overrides to a model instance, needed to correctly
     * display a given object.
     *
     * @param obj The object.
     * @param modelInstance The model instance.
     * @param materials The material source to use for loading replacement materials. Defaults to gameView.materials
     * @param models The model source to use for loading models. Defaults to gameView.models
     */
    Equipment.addRenderEquipment = function(obj, modelInstance, materials, models) {

        if (!materials)
            materials = gameView.materials;

        if (!models)
            models = gameView.models;

        modelInstance.clearOverrideMaterials();

        var equipment = Equipment.getRenderEquipment(obj);

        for (var materialId in equipment.materials) {
            var filename = equipment.materials[materialId];
            modelInstance.overrideMaterial(materialId, materials.load(filename));
        }

        equipment.meshes.forEach(function (filename) {
            modelInstance.addMesh(models.load(filename));
        });

        // Set the hair-color override
        if (obj.hairColor) {
            modelInstance.setMaterialPropertyVec4("hairColor", obj.hairColor);
        }
    };

    function load() {
        Equipment.register(readJson('equipment.js'));
    }

    StartupListeners.add(load, 'toee-equipment', []);

})();
