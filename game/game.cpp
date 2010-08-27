
#include <QCoreApplication>
#include <QSettings>
#include <QFileDialog>
#include <QFontDatabase>

#include "game.h"
#include "mainwindow.h"
#include "scriptengine.h"
#include "savegames.h"
#include "charactervault.h"

namespace EvilTemple {

    class GameData {
    public:
        ScriptEngine *scriptEngine;

        SaveGames *saveGames;
        CharacterVault *characterVault;
    };

    Game::Game(QObject *parent) :
        QObject(parent),
        d_ptr(new GameData)
    {
        d_ptr->scriptEngine = new ScriptEngine(this);
        d_ptr->scriptEngine->setObjectName("scriptEngine");

        d_ptr->characterVault = new CharacterVault("characters/", "userCharacters/",
                                                   d_ptr->scriptEngine->engine(), this);

        d_ptr->scriptEngine->exposeQObject("charactervault", d_ptr->characterVault);

        d_ptr->saveGames = new SaveGames("saves/", d_ptr->scriptEngine->engine(), this);

        d_ptr->scriptEngine->exposeQObject("savegames", d_ptr->saveGames);
    }

    Game::~Game() {
    }

    bool Game::start() {
        scriptEngine()->loadScripts();

        return true;
    }

    ScriptEngine *Game::scriptEngine() const
    {
        return d_ptr->scriptEngine;
    }

}
