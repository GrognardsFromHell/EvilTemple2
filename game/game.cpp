
#include <QDesktopServices>
#include <QCoreApplication>
#include <QDir>
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

        QDir dataPath;
        QDir userDataPath;
    };

    Game::Game(QObject *parent) :
        QObject(parent),
        d(new GameData)
    {
        // Deduce the data path
        QCoreApplication *app = QCoreApplication::instance();
        d->dataPath = app->applicationDirPath();
        d->dataPath.cd("data");

        QDir documentsDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
        d->userDataPath = documentsDir.absoluteFilePath("EvilTemple");
    }

    Game::~Game()
    {
    }

    bool Game::start()
    {
        qDebug("Using data path: %s.", qPrintable(d->dataPath.absolutePath()));

        // Create user data path if it doesn't exist
        if (!d->userDataPath.exists()) {
            qDebug("Creating the user data path: %s", qPrintable(d->userDataPath.absolutePath()));

            QDir root = d->userDataPath.root();
            root.mkpath(d->userDataPath.absolutePath());
        } else {
            qDebug("Using user data path: %s", qPrintable(d->userDataPath.absolutePath()));
        }

        qDebug("Initializing script engine.");
        d->scriptEngine = new ScriptEngine(this);
        d->scriptEngine->setObjectName("scriptEngine");

        qDebug("Initializing character vault");
        QString systemCharacters = "data/characters/"; // This path should be relative to enable lookup in the ZIPs
        qDebug("%s", qPrintable(d->userDataPath.absoluteFilePath("characters")));
        QString userCharacters = d->userDataPath.absoluteFilePath("characters");
        d->characterVault = new CharacterVault(systemCharacters, userCharacters, d->scriptEngine->engine(), this);
        d->scriptEngine->exposeQObject("charactervault", d->characterVault);

        QString savePath = d->userDataPath.absoluteFilePath("saves") + QDir::separator();
        qDebug("Initializing savegames (%s)", qPrintable(savePath));
        d->saveGames = new SaveGames(savePath, d->scriptEngine->engine(), this);
        d->scriptEngine->exposeQObject("savegames", d->saveGames);

        qDebug("Loading scripts");
        scriptEngine()->loadScripts();

        return true;
    }

    ScriptEngine *Game::scriptEngine() const
    {
        return d->scriptEngine;
    }

    const QString &Game::dataPath() const
    {
        return d->dataPath.absolutePath();
    }

    void Game::setDataPath(const QString &dataPath)
    {
        d->dataPath = dataPath;
    }

    const QString &Game::userDataPath() const
    {
        return d->userDataPath.absolutePath();
    }

    void Game::setUserDataPath(const QString &userDataPath)
    {
        d->userDataPath = userDataPath;
    }

}
