/**
 * Gives access to the various object prototypes that are used as templates for critters and objects
 * throughout the world.
 *
 * In JavaScript they are implemented as simple objects that can be used as prototypes for actual
 * object instances. The prototypes themselves have one of the many scriptable base-classes as their
 * prototype, which all have BaseObject as their root prototype.
 */
var Prototypes = (function() {

    var prototypes = {};

    /**
     * Loads the prototypes from the data files. This needs to be delay-loaded since it
     * depends on the object model to be present.
     */
    function load() {
        prototypes = eval('(' + readFile('prototypes.js') + ')');

        print("Loaded " + countProperties(prototypes) + " prototypes.");

        // Adds a pseudo-prototype for static map geometry
        prototypes['StaticGeometry'] = {
            interactive: false
        };

        // Assign the prototype of each loaded prototype
        for (var k in prototypes) {
            switch (prototypes[k].type) {
                case 'MapChanger':
                    prototypes[k].__proto__ = MapChanger;
                    break;
                case 'Portal':
                    prototypes[k].__proto__ = Portal;
                    break;
                case 'Container':
                    prototypes[k].__proto__ = Container;
                    break;
                case 'Scenery':
                    prototypes[k].__proto__ = Scenery;
                    break;
                case 'NonPlayerCharacter':
                    prototypes[k].__proto__ = NonPlayerCharacter;
                    break;
                case 'PlayerCharacter':
                    prototypes[k].__proto__ = PlayerCharacter;
                    break;
                default:
                    prototypes[k].__proto__ = BaseObject;
                    break;
            }
        }
    }

    StartupListeners.add(load, 'prototypes', ['objectmodel']);

    return {
        /**
         * Creates an object from a prototype.
         *
         * The resulting object will have the id property set to a fresh GUID.
         *
         * @param prototypeId The id of the prototype to use. If no such prototype exists, an exception will
         * be thrown.
         */
        createObject: function(prototypeId) {

            var prototype = prototypes[prototypeId];

            if (!prototype) {
                throw "Trying to create object with unknown prototype: " + prototypeId;
            }

            var object = {
                id: generateGuid(),
                prototype: prototypeId
            };

            /*
             TODO Fix this so the constructor property is set correctly. 
             This uses JavaScriptCore specific functionality and in its current state will not allow the use
             of instanceof.
             */
            object.__proto__ = prototype;

            return object;
        },

        /**
         * Reconnects an existing object to its prototype, based on its prototype property.
         * This also makes some assumptions about the object structure and reconnects nested objects
         * that also have a prototype property.
         *
         * @param object The object that will be reconnected.
         */
        reconnect: function(object) {

            if (object.prototype !== undefined) {
                object.__proto__ = prototypes[object.prototype];
            } else {
                object.__proto__ = BaseObject;
            }

            if (object.content !== undefined) {
                object.content.forEach(Prototypes.reconnect);
            }

        }
    };

})();
