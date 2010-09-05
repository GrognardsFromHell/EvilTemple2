/*
 This script adds the ToEE hairstyles to the equipment system.
 */
(function() {

    function getRecord(type, raceId, subType) {
        return {
            meshes: ['meshes/hair/' + type + '/' + raceId + '/' + subType + '.model']
        };
    }

    var data = {
        'hair-long': {
            'human-male': getRecord('long', 'human-male', 'small'),
            'human-female': getRecord('long', 'human-female', 'small'),
            // 'human-male': getRecord('long', 'human-male', 'big'), (Also exists)
            'dwarf-male': getRecord('long', 'dwarf-male', 'small'),
            'dwarf-female': getRecord('long', 'dwarf-female', 'small'),
            // 'dwarf-female': getRecord('long', 'dwarf-female', 'big'), (Also exists)
            'halforc-male': getRecord('long', 'halforc-male', 'small'),
            'halforc-female': getRecord('long', 'halforc-female', 'small')
            // 'halforc-female': getRecord('long', 'halforc-female', 'big'), (Also exists)
        },
        'hair-ponytail': {
            'human-male': getRecord('ponytail', 'human-male', 'small'),
            'human-female': getRecord('ponytail', 'human-female', 'small'),
            'dwarf-male': getRecord('ponytail', 'dwarf-male', 'small'),
            'dwarf-female': getRecord('ponytail', 'dwarf-female', 'small'),
            'halforc-male': getRecord('ponytail', 'halforc-male', 'small'),
            'halforc-female': getRecord('ponytail', 'halforc-female', 'small')
        },
        'hair-short': {
            'human-male': getRecord('short', 'human-male', 'small'),
            'human-female': getRecord('short', 'human-female', 'small'),
            'dwarf-male': getRecord('short', 'dwarf-male', 'small'),
            'dwarf-female': getRecord('short', 'dwarf-female', 'small'),
            'halforc-male': getRecord('short', 'halforc-male', 'small'),
            'halforc-female': getRecord('short', 'halforc-female', 'small')
        },
        'hair-topknot': {
            'human-male': getRecord('topknot', 'human-male', 'small'),
            'human-female': getRecord('topknot', 'human-female', 'small'),
            'dwarf-male': getRecord('topknot', 'dwarf-male', 'small'),
            'dwarf-female': getRecord('topknot', 'dwarf-female', 'small'),
            'halforc-male': getRecord('topknot', 'halforc-male', 'small'),
            'halforc-female': getRecord('topknot', 'halforc-female', 'small')

            /*
             These also exist:
             'human-male': getRecord('topknot', 'human-male', 'big'),
             'human-female': getRecord('topknot', 'human-female', 'big'),
             'dwarf-male': getRecord('topknot', 'dwarf-male', 'big'),
             'dwarf-female': getRecord('topknot', 'dwarf-female', 'big'),
             'halforc-male': getRecord('topknot', 'halforc-male', 'big'),
             'halforc-female': getRecord('topknot', 'halforc-female', 'big')*/
        },
        'hair-pigtails': {
            'human-female': getRecord('pigtails', 'human-female', 'small'),
            // 'human-male': getRecord('pigtails', 'human-male', 'big'), (Also exists)
            'dwarf-female': getRecord('pigtails', 'dwarf-female', 'small'),
            // 'dwarf-female': getRecord('pigtails', 'dwarf-female', 'big'), (Also exists)
            'halforc-female': getRecord('pigtails', 'halforc-female', 'small'),
            // 'halforc-female': getRecord('pigtails', 'halforc-female', 'big'), (Also exists)

            // Reuse the female models for males
            'human-male': getRecord('pigtails', 'human-female', 'small'),
            'dwarf-male': getRecord('pigtails', 'dwarf-female', 'small'),
            'halforc-male': getRecord('pigtails', 'halforc-female', 'small')
        },
        'hair-braids': {
            'human-female': getRecord('braids', 'human-female', 'small'),
            // 'human-male': getRecord('braids', 'human-male', 'none'), (Also exists)
            'dwarf-female': getRecord('braids', 'dwarf-female', 'small'),
            // 'dwarf-female': getRecord('braids', 'dwarf-female', 'none'), (Also exists)
            'halforc-female': getRecord('braids', 'halforc-female', 'small'),
            // 'halforc-female': getRecord('braids', 'halforc-female', 'none'), (Also exists)

            // Reuse the female models for males
            'human-male': getRecord('braids', 'human-female', 'small'),
            'dwarf-male': getRecord('braids', 'dwarf-female', 'small'),
            'halforc-male': getRecord('braids', 'halforc-female', 'small')
        },
        'hair-mohawk': {
            'human-male': getRecord('mohawk', 'human-male', 'small'),
            'human-female': getRecord('mohawk', 'human-female', 'small'),
            'dwarf-male': getRecord('mohawk', 'dwarf-male', 'small'),
            'dwarf-female': getRecord('mohawk', 'dwarf-female', 'small'),
            'halforc-male': getRecord('mohawk', 'halforc-male', 'small'),
            'halforc-female': getRecord('mohawk', 'halforc-female', 'small')

            /*
             These also exist:
             'human-male': getRecord('mohawk', 'human-male', 'big'),
             'human-female': getRecord('mohawk', 'human-female', 'big'),
             'dwarf-male': getRecord('mohawk', 'dwarf-male', 'big'),
             'dwarf-female': getRecord('mohawk', 'dwarf-female', 'big'),
             'halforc-male': getRecord('mohawk', 'halforc-male', 'big'),
             'halforc-female': getRecord('mohawk', 'halforc-female', 'big')*/
        },
        'hair-ponytail2': {
            'human-female': getRecord('ponytail2', 'human-female', 'small'),
            'dwarf-female': getRecord('ponytail2', 'dwarf-female', 'small'),
            'halforc-female': getRecord('ponytail2', 'halforc-female', 'small'),

            // Reuse the female models for males
            'human-male': getRecord('ponytail2', 'human-female', 'small'),
            'dwarf-male': getRecord('ponytail2', 'dwarf-female', 'small'),
            'halforc-male': getRecord('ponytail2', 'halforc-female', 'small')
        },
        'hair-mullet': {
            'human-male': getRecord('mullet', 'human-male', 'small'),
            'dwarf-male': getRecord('mullet', 'dwarf-male', 'small'),
            'halforc-male': getRecord('mullet', 'halforc-male', 'small'),

            // Reuse the male models for females
            'human-female': getRecord('mullet', 'human-male', 'small'),
            'dwarf-female': getRecord('mullet', 'dwarf-male', 'small'),
            'halforc-female': getRecord('mullet', 'halforc-male', 'small')
        },
        'hair-bald': {
            'human-male': getRecord('bald', 'human-male', 'none'),
            'dwarf-male': getRecord('bald', 'dwarf-male', 'none'),
            'halforc-male': getRecord('bald', 'halforc-male', 'none'),

            // Reuse the male models for females
            'human-female': getRecord('bald', 'human-male', 'none'),
            'dwarf-female': getRecord('bald', 'dwarf-male', 'none'),
            'halforc-female': getRecord('bald', 'halforc-male', 'none')
        },
        'hair-medium': {
            'human-male': getRecord('medium', 'human-male', 'small'),
            'dwarf-male': getRecord('medium', 'dwarf-male', 'small'),
            'halforc-male': getRecord('medium', 'halforc-male', 'small'),

            // Reuse the male models for females
            'human-female': getRecord('medium', 'human-male', 'small'),
            'dwarf-female': getRecord('medium', 'dwarf-male', 'small'),
            'halforc-female': getRecord('medium', 'halforc-male', 'small')
        }
    };

    function register() {
        Equipment.register(data);
    }

    StartupListeners.add(register, "toee-hairstyles", []);

})();
