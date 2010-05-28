
#include <QCoreApplication>
#include <QSettings>
#include <QFileDialog>

#include "game.h"
#include "mainwindow.h"
#include "scriptengine.h"

namespace EvilTemple {

    class GameData {
    public:
        ScriptEngine *scriptEngine;
    };

    Game::Game(QObject *parent) :
        QObject(parent),
        d_ptr(new GameData)
    {
        d_ptr->scriptEngine = new ScriptEngine(this);
        d_ptr->scriptEngine->setObjectName("scriptEngine");
    }

    Game::~Game() {
    }

    bool Game::start() {
        scriptEngine()->loadScripts();

        scriptEngine()->callGlobalFunction("startup");

        return true;
    }

    ScriptEngine *Game::scriptEngine() const
    {
        return d_ptr->scriptEngine;
    }

}
