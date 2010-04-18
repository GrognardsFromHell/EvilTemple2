
#include <QCoreApplication>
#include <QSettings>
#include <QFileDialog>

#include "game.h"
#include "ui/mainwindow.h"
#include "io/basepathfinder.h"
#include "camera.h"
#include "scriptengine.h"

namespace EvilTemple {

    class GameData {
    public:
        Camera *camera;
        ScriptEngine *scriptEngine;
    };

    Game::Game(QObject *parent) :
        QObject(parent),
        d_ptr(new GameData)
    {
        // Set default values for org+app so QSettings uses them everywhere
        QCoreApplication::setOrganizationName("Sebastian Hartte");
        QCoreApplication::setOrganizationDomain("toee.hartte.de");
        QCoreApplication::setApplicationName("EvilTemple");

        d_ptr->camera = new Camera(this);
        d_ptr->camera->setObjectName("camera");

        // d_ptr->cursors = new Cursors(d_ptr->vfs, this);
        // d_ptr->cursors->setObjectName("cursors");

        d_ptr->scriptEngine = new ScriptEngine(this);
        d_ptr->scriptEngine->setObjectName("scriptEngine");
    }

    Game::~Game() {
    }

    bool Game::start() {

        return true;
    }

    Camera *Game::camera() const {
        return d_ptr->camera;
    }

    ScriptEngine *Game::scriptEngine() const
    {
        return d_ptr->scriptEngine;
    }

}
