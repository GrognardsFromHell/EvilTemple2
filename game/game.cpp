
#include <QDesktopServices>
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QFontDatabase>

#include <common/paths.h>

#include "game.h"
#include "mainwindow.h"
#include "scriptengine.h"
#include "savegames.h"
#include "charactervault.h"
#include "translations.h"

namespace EvilTemple {

    class GameData {
    public:
        ScriptEngine *scriptEngine;

        SaveGames *saveGames;
        CharacterVault *characterVault;
        Translations translations;
        const Paths *paths;

        QDir dataPath;
    };

    Game::Game(const Paths *paths, QObject *parent) :
        QObject(parent), d(new GameData)
    {
        d->paths = paths;

        // Deduce the data path
        d->dataPath = d->paths->installationDir();
        if (!d->dataPath.cd("data")) {
            qWarning("Data directory in installation directory not found.");
        }
    }

    Game::~Game()
    {
    }

    bool Game::start()
    {
        qDebug("Using data path: %s.", qPrintable(d->dataPath.absolutePath()));

        qDebug("Initializing script engine.");
        d->scriptEngine = new ScriptEngine(this);
        d->scriptEngine->setObjectName("scriptEngine");

        if (!d->translations.load("translation.dat")) {
            qFatal("Unable to load translations.");
        }
        d->scriptEngine->exposeQObject("translations", &d->translations);

        qDebug("Initializing character vault");
        QString systemCharacters = "data/characters/"; // This path should be relative to enable lookup in the ZIPs
        QString userCharacters = d->paths->userDataDir().absoluteFilePath("characters");
        d->characterVault = new CharacterVault(systemCharacters, userCharacters, d->scriptEngine->engine(), this);
        d->scriptEngine->exposeQObject("charactervault", d->characterVault);

        QString savePath = d->paths->userDataDir().absoluteFilePath("saves") + QDir::separator();
        qDebug("Initializing savegames (%s)", qPrintable(savePath));
        d->saveGames = new SaveGames(savePath, d->scriptEngine->engine(), this);
        d->scriptEngine->exposeQObject("savegames", d->saveGames);

        qDebug("Loading scripts");
        scriptEngine()->loadScripts(d->dataPath.filePath("scripts"));

        return true;
    }

    ScriptEngine *Game::scriptEngine() const
    {
        return d->scriptEngine;
    }

    const Translations *Game::translations() const
    {
        return &d->translations;
    }

    const Paths *Game::paths() const
    {
        return d->paths;
    }

}
