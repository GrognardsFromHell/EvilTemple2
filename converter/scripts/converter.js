
var PixelPerWorldTile = 28.2842703;

// Maps legacy map id to map name (e.g. 5001 -> Map-2-Hommlet-Exterior)
var LegacyMapList = {};

var npcAddMeshes = {}; // Maps npc addmesh ids to the actual addmeshes.

/**
   Called for stairs-down, stairs-up and other teleport icons.
   ALL have [ "SeeThrough", "ShootThrough", "NoBlock", "ProvidesCover", "Invulnerable" ]
  */
function teleportIcon(proto) {
        proto.type = 'MapChanger';
        proto.flags = ["SeeThrough", "ShootThrough", "NoBlock", "ProvidesCover", "Invulnerable"];
}

function money(proto) {
    proto.flags = ["Flat", "SeeThrough", "ShootThrough", "NoBlock"];
}

var processors = {};
processors.register = function(func, ids) {
        for (var i = 0; i < ids.length; ++i)
                this[ids[i]] = func;
};

processors.register(money, ['7000', '7001', '7002', '7003']);
processors.register(teleportIcon, ['2011', '2012', '2013', '2014', '2015', '2035', '2036', '2037', '2038', '2039']);

var hairTypes = {
        "Longhair (m/f)": 0,
        "Ponytail (m/f)": 1,
        "Shorthair (m/f)": 2,
        "Topknot (m/f)": 3,
        "Mullet (m)": 4,
        "Pigtails (f)": 5,
        "Bald (m)": 6,
        "Braids (f)": 7,
        "Mohawk (m/f)": 8,
        "Medium (m)": 9,
        "Ponytail2 (f)": 10
};

var hairColors = {
        "Black": 0,
        "Blonde": 1,
        "Blue": 2,
        "Brown": 3,
        "Light Brown": 4,
        "Pink": 5,
        "Red": 6,
        "White": 7
};

function postprocess(prototypes) {

        processLegacyMapList();
        processJumpPoints();
        processReputations();
        processAddMeshes();
        processPortraits();
        processInventory();

        print("Running postprocessor on: " + prototypes);

        for (var k in prototypes) {
                var processor = processors[k];
                var proto = prototypes[k];
                if (processor !== undefined) {
                        processor(proto);
                }

                // Generic post-processing
                if (proto['wearMeshId'] !== undefined) {
                        proto['wearMeshId'] /= 100; // the last two digits are the specific model-types, but we organize differently
                }
                if (proto['hairType'] !== undefined) {
                        var hairId = hairTypes[proto['hairType']];
                        if (hairId === undefined) {
                                print("Unknown hair style: " + proto['hairType']);
                                proto['hairType'] = undefined;
                        } else {
                                proto['hairType'] = hairId;
                        }
                }
                if (proto['hairColor'] !== undefined) {
                        var hairId = hairColors[proto['hairColor']];
                        if (hairId === undefined) {
                                print("Unknown hair color: " + proto['hairColor']);
                                proto['hairColor'] = undefined;
                        } else {
                                proto['hairColor'] = hairId;
                        }
                }
                if (proto['portraitId'] !== undefined) {
                        var portraitId = proto['portraitId'];
                        delete proto['portraitId'];
                        proto['portrait'] = Math.floor(portraitId / 10);
                }

                var addMeshId = proto['addMeshId'];
                if (addMeshId !== undefined) {
                        delete proto['addMeshId'];
                        if (addMeshId < 10000000) {
                                print("Warning: Prototype has invalid add mesh id: " + addMeshId);
                        } else {
                                var filename = npcAddMeshes[addMeshId];
                                if (filename === undefined) {
                                        print ('Unknown addmesh id: ' + addMeshId);
                                } else {
                                        proto['addMeshes'] = [filename];
                                }
                        }
                }
        }

        var result = JSON.stringify(prototypes);
        addFile('prototypes.js', result, 9);

        print("Finished postprocessing");

        return result;
}

function processLegacyMapList() {
    var records = readMes('rules/MapList.mes');

    for (var mapId in records) {
        var mapDir = records[mapId].split(',')[0];
        LegacyMapList[mapId] = mapDir.toLowerCase();
    }
}

function mangleFilename(filename) {
        filename = filename.replace(/\\/g, '/');
        filename = filename.replace(/^art\/+/i, '');
        return filename;
}

function processAddMeshes() {
        var addMeshes = readMes('rules/addmesh.mes');

        var equipment = {};

        var typeIds = {};
        typeIds[0] = 'human-male';
        typeIds[1] = 'human-female';
        typeIds[2] = 'elf-male';
        typeIds[3] = 'elf-female';
        typeIds[4] = 'halforc-male';
        typeIds[5] = 'halforc-female';
        typeIds[6] = 'dwarf-male';
        typeIds[7] = 'dwarf-female';
        typeIds[8] = 'gnome-male';
        typeIds[9] = 'gnome-female';
        typeIds[10] = 'halfelf-male';
        typeIds[11] = 'halfelf-female';
        typeIds[12] = 'halfling-male';
        typeIds[13] = 'halfling-female';

        // Group by item-id
        for (var k in addMeshes) {
                if (k >= 10000000) {
                        var filename = addMeshes[k];
                        filename = filename.replace(/\\/g, '/');
                        filename = filename.replace(/^art\/+/i, '');
                        filename = filename.replace(/skm$/i, 'model');
                        npcAddMeshes[k] = filename;
                        continue;
                }

                var id = Math.floor(k / 100);
                var typeId = typeIds[k % 100];

                if (typeId === undefined) {
                        print("Unknown type id for addmesh: " + typeId);
                }

                if (equipment[id] === undefined)
                        equipment[id] = {};

                if (equipment[id][typeId] === undefined)
                        equipment[id][typeId] = {};

                var meshFilenames = addMeshes[k].split(';');

                for (var i = 0; i < meshFilenames.length; ++i) {
                        meshFilenames[i] = meshFilenames[i].replace(/\\/g, '/');
                        meshFilenames[i] = meshFilenames[i].replace(/^art\/+/i, '');
                        meshFilenames[i] = meshFilenames[i].replace(/skm$/i, 'model');
                }

                equipment[id][typeId].meshes = meshFilenames;
        }

        // Group by item-id
        var materials = readMes('rules/materials.mes');
        for (var k in materials) {
                var id = Math.floor(k / 100);
                var typeId = typeIds[k % 100];

                if (typeId === undefined) {
                        print("Unknown type id for material: " + typeId);
                }

                if (equipment[id] === undefined)
                        equipment[id] = {};

                if (equipment[id][typeId] === undefined)
                        equipment[id][typeId] = {};

                var line = materials[k].split(':');
                var slot = line[0];
                var material = mangleFilename(line[1]).replace(/mdf$/i, 'xml');

                if (equipment[id][typeId].materials === undefined)
                        equipment[id][typeId].materials = {};
                equipment[id][typeId].materials[slot] = material;
        }

        var result = JSON.stringify(equipment);
        addFile('equipment.js', result, 9);
}

function processJumpPoints() {
        var records = readTab('rules/jumppoint.tab');

        var jumppoints = {};

        for (var i = 0; i < records.length; ++i) {
                var record = records[i];

                var mapId = LegacyMapList[record[2]];

                if (mapId === undefined) {
                        print("Warning: Undefined map id in jumppoint " + record[0] + ": " + record[2]);
                        continue;
                }

                var jumppoint = {
                        name: record[1],
                        map: mapId,
                        position: [Math.round((parseInt(record[3]) + 0.5) * PixelPerWorldTile),
                                   0,
                                   Math.round((parseInt(record[4]) + 0.5) * PixelPerWorldTile)
                        ]
                };

                jumppoints[record[0]] = jumppoint;
        }

        var result = JSON.stringify(jumppoints);
        addFile('jumppoints.js', result, 9);
}

function processReputations() {
    var records = readMes('mes/gamereplog.mes');

    var reputations = {};

    for (var i = 0; i < 1000; ++i) {
        var name = records[i];
        if (!name)
            continue;

        reputations[i] = {
            'name': name,
            'description': records[1000 + i],
            'effect': records[2000 + i]
        };
    }

    var result = JSON.stringify(reputations);
    addFile('reputations.js', result, 9);
}

function processPortraits() {
        var records = readMes('art/interface/portraits/portraits.mes');

        var portraits = {};

        for (var k in records) {
                k = parseInt(k);

                if (k % 10 != 0)
                        continue;

                var large = records[k];
                if (large == '')
                        large = null;
                var medium = records[k + 1];
                var small = records[k + 2];
                var smallGray = records[k + 3];
                var mediumGray = records[k + 4];

                var files = [large, medium, small, mediumGray, smallGray];

                for (var i = 0; i < files.length; ++i) {
                        if (files[i] === undefined || files[i] === null)
                                continue;
                        files[i] = files[i].replace(/\.tga$/i, '.png');
                }

                portraits[k/10] = files;
        }

        var result = JSON.stringify(portraits);
        addFile('portraits.js', result, 9);
}

function processInventory() {
        var records = readMes('art/interface/inventory/inventory.mes');

        var inventory = {};

        for (var k in records) {
                inventory[k] = records[k].replace(/\.tga$/i, '.png');
        }

        var result = JSON.stringify(inventory);
        addFile('inventoryIcons.js', result, 9);
}

/*
    http://www.JSON.org/json2.js
    2010-03-20

    Public Domain.

    NO WARRANTY EXPRESSED OR IMPLIED. USE AT YOUR OWN RISK.

    See http://www.JSON.org/js.html


    This code should be minified before deployment.
    See http://javascript.crockford.com/jsmin.html

    USE YOUR OWN COPY. IT IS EXTREMELY UNWISE TO LOAD CODE FROM SERVERS YOU DO
    NOT CONTROL.


    This file creates a global JSON object containing two methods: stringify
    and parse.

        JSON.stringify(value, replacer, space)
            value       any JavaScript value, usually an object or array.

            replacer    an optional parameter that determines how object
                        values are stringified for objects. It can be a
                        function or an array of strings.

            space       an optional parameter that specifies the indentation
                        of nested structures. If it is omitted, the text will
                        be packed without extra whitespace. If it is a number,
                        it will specify the number of spaces to indent at each
                        level. If it is a string (such as '\t' or '&nbsp;'),
                        it contains the characters used to indent at each level.

            This method produces a JSON text from a JavaScript value.

            When an object value is found, if the object contains a toJSON
            method, its toJSON method will be called and the result will be
            stringified. A toJSON method does not serialize: it returns the
            value represented by the name/value pair that should be serialized,
            or undefined if nothing should be serialized. The toJSON method
            will be passed the key associated with the value, and this will be
            bound to the value

            For example, this would serialize Dates as ISO strings.

                Date.prototype.toJSON = function (key) {
                    function f(n) {
                        // Format integers to have at least two digits.
                        return n < 10 ? '0' + n : n;
                    }

                    return this.getUTCFullYear()   + '-' +
                         f(this.getUTCMonth() + 1) + '-' +
                         f(this.getUTCDate())      + 'T' +
                         f(this.getUTCHours())     + ':' +
                         f(this.getUTCMinutes())   + ':' +
                         f(this.getUTCSeconds())   + 'Z';
                };

            You can provide an optional replacer method. It will be passed the
            key and value of each member, with this bound to the containing
            object. The value that is returned from your method will be
            serialized. If your method returns undefined, then the member will
            be excluded from the serialization.

            If the replacer parameter is an array of strings, then it will be
            used to select the members to be serialized. It filters the results
            such that only members with keys listed in the replacer array are
            stringified.

            Values that do not have JSON representations, such as undefined or
            functions, will not be serialized. Such values in objects will be
            dropped; in arrays they will be replaced with null. You can use
            a replacer function to replace those with JSON values.
            JSON.stringify(undefined) returns undefined.

            The optional space parameter produces a stringification of the
            value that is filled with line breaks and indentation to make it
            easier to read.

            If the space parameter is a non-empty string, then that string will
            be used for indentation. If the space parameter is a number, then
            the indentation will be that many spaces.

            Example:

            text = JSON.stringify(['e', {pluribus: 'unum'}]);
            // text is '["e",{"pluribus":"unum"}]'


            text = JSON.stringify(['e', {pluribus: 'unum'}], null, '\t');
            // text is '[\n\t"e",\n\t{\n\t\t"pluribus": "unum"\n\t}\n]'

            text = JSON.stringify([new Date()], function (key, value) {
                return this[key] instanceof Date ?
                    'Date(' + this[key] + ')' : value;
            });
            // text is '["Date(---current time---)"]'


        JSON.parse(text, reviver)
            This method parses a JSON text to produce an object or array.
            It can throw a SyntaxError exception.

            The optional reviver parameter is a function that can filter and
            transform the results. It receives each of the keys and values,
            and its return value is used instead of the original value.
            If it returns what it received, then the structure is not modified.
            If it returns undefined then the member is deleted.

            Example:

            // Parse the text. Values that look like ISO date strings will
            // be converted to Date objects.

            myData = JSON.parse(text, function (key, value) {
                var a;
                if (typeof value === 'string') {
                    a =
/^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2}(?:\.\d*)?)Z$/.exec(value);
                    if (a) {
                        return new Date(Date.UTC(+a[1], +a[2] - 1, +a[3], +a[4],
                            +a[5], +a[6]));
                    }
                }
                return value;
            });

            myData = JSON.parse('["Date(09/09/2001)"]', function (key, value) {
                var d;
                if (typeof value === 'string' &&
                        value.slice(0, 5) === 'Date(' &&
                        value.slice(-1) === ')') {
                    d = new Date(value.slice(5, -1));
                    if (d) {
                        return d;
                    }
                }
                return value;
            });


    This is a reference implementation. You are free to copy, modify, or
    redistribute.
*/

/*jslint evil: true, strict: false */

/*members "", "\b", "\t", "\n", "\f", "\r", "\"", JSON, "\\", apply,
    call, charCodeAt, getUTCDate, getUTCFullYear, getUTCHours,
    getUTCMinutes, getUTCMonth, getUTCSeconds, hasOwnProperty, join,
    lastIndex, length, parse, prototype, push, replace, slice, stringify,
    test, toJSON, toString, valueOf
*/


// Create a JSON object only if one does not already exist. We create the
// methods in a closure to avoid creating global variables.

if (!this.JSON) {
    this.JSON = {};
}

(function () {

    function f(n) {
        // Format integers to have at least two digits.
        return n < 10 ? '0' + n : n;
    }

    if (typeof Date.prototype.toJSON !== 'function') {

        Date.prototype.toJSON = function (key) {

            return isFinite(this.valueOf()) ?
                   this.getUTCFullYear()   + '-' +
                 f(this.getUTCMonth() + 1) + '-' +
                 f(this.getUTCDate())      + 'T' +
                 f(this.getUTCHours())     + ':' +
                 f(this.getUTCMinutes())   + ':' +
                 f(this.getUTCSeconds())   + 'Z' : null;
        };

        String.prototype.toJSON =
        Number.prototype.toJSON =
        Boolean.prototype.toJSON = function (key) {
            return this.valueOf();
        };
    }

    var cx = /[\u0000\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,
        escapable = /[\\\"\x00-\x1f\x7f-\x9f\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,
        gap,
        indent,
        meta = {    // table of character substitutions
            '\b': '\\b',
            '\t': '\\t',
            '\n': '\\n',
            '\f': '\\f',
            '\r': '\\r',
            '"' : '\\"',
            '\\': '\\\\'
        },
        rep;


    function quote(string) {

// If the string contains no control characters, no quote characters, and no
// backslash characters, then we can safely slap some quotes around it.
// Otherwise we must also replace the offending characters with safe escape
// sequences.

        escapable.lastIndex = 0;
        return escapable.test(string) ?
            '"' + string.replace(escapable, function (a) {
                var c = meta[a];
                return typeof c === 'string' ? c :
                    '\\u' + ('0000' + a.charCodeAt(0).toString(16)).slice(-4);
            }) + '"' :
            '"' + string + '"';
    }


    function str(key, holder) {

// Produce a string from holder[key].

        var i,          // The loop counter.
            k,          // The member key.
            v,          // The member value.
            length,
            mind = gap,
            partial,
            value = holder[key];

// If the value has a toJSON method, call it to obtain a replacement value.

        if (value && typeof value === 'object' &&
                typeof value.toJSON === 'function') {
            value = value.toJSON(key);
        }

// If we were called with a replacer function, then call the replacer to
// obtain a replacement value.

        if (typeof rep === 'function') {
            value = rep.call(holder, key, value);
        }

// What happens next depends on the value's type.

        switch (typeof value) {
        case 'string':
            return quote(value);

        case 'number':

// JSON numbers must be finite. Encode non-finite numbers as null.

            return isFinite(value) ? String(value) : 'null';

        case 'boolean':
        case 'null':

// If the value is a boolean or null, convert it to a string. Note:
// typeof null does not produce 'null'. The case is included here in
// the remote chance that this gets fixed someday.

            return String(value);

// If the type is 'object', we might be dealing with an object or an array or
// null.

        case 'object':

// Due to a specification blunder in ECMAScript, typeof null is 'object',
// so watch out for that case.

            if (!value) {
                return 'null';
            }

// Make an array to hold the partial results of stringifying this object value.

            gap += indent;
            partial = [];

// Is the value an array?

            if (Object.prototype.toString.apply(value) === '[object Array]') {

// The value is an array. Stringify every element. Use null as a placeholder
// for non-JSON values.

                length = value.length;
                for (i = 0; i < length; i += 1) {
                    partial[i] = str(i, value) || 'null';
                }

// Join all of the elements together, separated with commas, and wrap them in
// brackets.

                v = partial.length === 0 ? '[]' :
                    gap ? '[\n' + gap +
                            partial.join(',\n' + gap) + '\n' +
                                mind + ']' :
                          '[' + partial.join(',') + ']';
                gap = mind;
                return v;
            }

// If the replacer is an array, use it to select the members to be stringified.

            if (rep && typeof rep === 'object') {
                length = rep.length;
                for (i = 0; i < length; i += 1) {
                    k = rep[i];
                    if (typeof k === 'string') {
                        v = str(k, value);
                        if (v) {
                            partial.push(quote(k) + (gap ? ': ' : ':') + v);
                        }
                    }
                }
            } else {

// Otherwise, iterate through all of the keys in the object.

                for (k in value) {
                    if (Object.hasOwnProperty.call(value, k)) {
                        v = str(k, value);
                        if (v) {
                            partial.push(quote(k) + (gap ? ': ' : ':') + v);
                        }
                    }
                }
            }

// Join all of the member texts together, separated with commas,
// and wrap them in braces.

            v = partial.length === 0 ? '{}' :
                gap ? '{\n' + gap + partial.join(',\n' + gap) + '\n' +
                        mind + '}' : '{' + partial.join(',') + '}';
            gap = mind;
            return v;
        }
    }

// If the JSON object does not yet have a stringify method, give it one.

    if (typeof JSON.stringify !== 'function') {
        JSON.stringify = function (value, replacer, space) {

// The stringify method takes a value and an optional replacer, and an optional
// space parameter, and returns a JSON text. The replacer can be a function
// that can replace values, or an array of strings that will select the keys.
// A default replacer method can be provided. Use of the space parameter can
// produce text that is more easily readable.

            var i;
            gap = '';
            indent = '';

// If the space parameter is a number, make an indent string containing that
// many spaces.

            if (typeof space === 'number') {
                for (i = 0; i < space; i += 1) {
                    indent += ' ';
                }

// If the space parameter is a string, it will be used as the indent string.

            } else if (typeof space === 'string') {
                indent = space;
            }

// If there is a replacer, it must be a function or an array.
// Otherwise, throw an error.

            rep = replacer;
            if (replacer && typeof replacer !== 'function' &&
                    (typeof replacer !== 'object' ||
                     typeof replacer.length !== 'number')) {
                throw new Error('JSON.stringify');
            }

// Make a fake root object containing our value under the key of ''.
// Return the result of stringifying the value.

            return str('', {'': value});
        };
    }


// If the JSON object does not yet have a parse method, give it one.

    if (typeof JSON.parse !== 'function') {
        JSON.parse = function (text, reviver) {

// The parse method takes a text and an optional reviver function, and returns
// a JavaScript value if the text is a valid JSON text.

            var j;

            function walk(holder, key) {

// The walk method is used to recursively walk the resulting structure so
// that modifications can be made.

                var k, v, value = holder[key];
                if (value && typeof value === 'object') {
                    for (k in value) {
                        if (Object.hasOwnProperty.call(value, k)) {
                            v = walk(value, k);
                            if (v !== undefined) {
                                value[k] = v;
                            } else {
                                delete value[k];
                            }
                        }
                    }
                }
                return reviver.call(holder, key, value);
            }


// Parsing happens in four stages. In the first stage, we replace certain
// Unicode characters with escape sequences. JavaScript handles many characters
// incorrectly, either silently deleting them, or treating them as line endings.

            text = String(text);
            cx.lastIndex = 0;
            if (cx.test(text)) {
                text = text.replace(cx, function (a) {
                    return '\\u' +
                        ('0000' + a.charCodeAt(0).toString(16)).slice(-4);
                });
            }

// In the second stage, we run the text against regular expressions that look
// for non-JSON patterns. We are especially concerned with '()' and 'new'
// because they can cause invocation, and '=' because it can cause mutation.
// But just to be safe, we want to reject all unexpected forms.

// We split the second stage into 4 regexp operations in order to work around
// crippling inefficiencies in IE's and Safari's regexp engines. First we
// replace the JSON backslash pairs with '@' (a non-JSON character). Second, we
// replace all simple value tokens with ']' characters. Third, we delete all
// open brackets that follow a colon or comma or that begin the text. Finally,
// we look to see that the remaining characters are only whitespace or ']' or
// ',' or ':' or '{' or '}'. If that is so, then the text is safe for eval.

            if (/^[\],:{}\s]*$/.
test(text.replace(/\\(?:["\\\/bfnrt]|u[0-9a-fA-F]{4})/g, '@').
replace(/"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g, ']').
replace(/(?:^|:|,)(?:\s*\[)+/g, ''))) {

// In the third stage we use the eval function to compile the text into a
// JavaScript structure. The '{' operator is subject to a syntactic ambiguity
// in JavaScript: it can begin a block or an object literal. We wrap the text
// in parens to eliminate the ambiguity.

                j = eval('(' + text + ')');

// In the optional fourth stage, we recursively walk the new structure, passing
// each name/value pair to a reviver function for possible transformation.

                return typeof reviver === 'function' ?
                    walk({'': j}, '') : j;
            }

// If the text is not JSON parseable, then a SyntaxError is thrown.

            throw new SyntaxError('JSON.parse');
        };
    }
}());
