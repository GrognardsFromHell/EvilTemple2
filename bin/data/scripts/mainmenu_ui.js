/**
 * Manages the main menu UI overlay.
 */
var MainMenuUi = {};

(function () {

    var mainMenu = null;

    function startGame() {
        showDebugBar();
        PartyUi.show();

        // Set up a nice debugging party
        Party.money.addGold(1000); // Start with 1000 gold

        var player1 = {
            __proto__: PlayerCharacter,
            id: generateGuid(),
            prototype: 13000,
            position: [0, 0, 0],
            portrait: '151',
            name: 'Storm'
        };
        connectToPrototype(player1);
        Party.addMember(player1);

        var startMap;

        // Vignette based on party alignment
        switch (Party.alignment) {
            case Alignment.LawfulGood:
                startMap = Maps.mapsById['vignette-lawful-good'];
                startMovie = 'movies/New_LG_Final_0.bik';
                break;
            case Alignment.NeutralGood:
                startMap = Maps.mapsById['vignette-good'];
                startMovie = 'movies/New_NG_Final_0.bik';
                break;
            case Alignment.ChaoticGood:
                startMap = Maps.mapsById['vignette-chaotic-good'];
                startMovie = 'movies/New_CG_Final_0.bik';
                break;
            case Alignment.LawfulNeutral:
                startMap = Maps.mapsById['vignette-lawful'];
                startMovie = 'movies/New_LN_Final_0.bik';
                break;
            case Alignment.TrueNeutral:
                startMap = Maps.mapsById['vignette-neutral'];
                startMovie = 'movies/New_NN_Final_0.bik';
                break;
            case Alignment.ChaoticNeutral:
                startMap = Maps.mapsById['vignette-chaotic'];
                startMovie = 'movies/New_CN_Final_0.bik';
                break;
            case Alignment.LawfulEvil:
                startMap = Maps.mapsById['vignette-lawful-evil'];
                startMovie = 'movies/New_CE-intro.bik';
                break;
            case Alignment.NeutralEvil:
                startMap = Maps.mapsById['vignette-evil'];
                startMovie = 'movies/New_NE_FInal0.bik';
                break;
            case Alignment.ChaoticEvil:
                startMap = Maps.mapsById['vignette-chaotic-evil'];
                startMovie = 'movies/New_LE_Final_0.bik';
                break;
        }

        if (startMovie) {
            if (gameView.playMovie(startMovie, function() {
                if (startMap) {
                    Maps.goToMap(startMap, startMap.startPosition);
                } else {
                    print("Unknown start map for alignment: " + Party.alignment);
                }
            })) {
                return;
            }
        }

        if (startMap) {
            Maps.goToMap(startMap, startMap.startPosition);
        } else {
            print("Unknown start map for alignment: " + Party.alignment);
        }
    }

    MainMenuUi.show = function() {
        if (mainMenu)
            return;

        mainMenu = gameView.showView("interface/MainMenu.qml");
        mainMenu.newGameClicked.connect(function() {
            mainMenu.deleteLater();
            mainMenu = null;

            startGame();
        });
    };

})();
