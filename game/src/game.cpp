
#include <QCoreApplication>
#include <QSettings>
#include <QFileDialog>

#include "game.h"
#include "material.h"
#include "materials.h"
#include "ui/mainwindow.h"
#include "io/virtualfilesystem.h"
#include "io/troikaarchive.h"
#include "io/basepathfinder.h"
#include "ui/cursors.h"
#include "camera.h"
#include "campaign/campaign.h"
#include "zonetemplates.h"
#include "prototypes.h"
#include "models.h"
#include "scriptengine.h"

namespace EvilTemple {

    class GameData {
    public:
        VirtualFileSystem *vfs;
        Cursors *cursors;
        Materials *materials;
        Camera *camera;
        ZoneTemplates *zoneTemplates;
        Campaign *campaign;
        Prototypes *prototypes;
        Models *models;
        ScriptEngine *scriptEngine;

        QString getDefaultBasePath() {
            return BasepathFinder::find().absolutePath();
        }

        bool validateBasedir(const QDir &basedir) {
            if (!basedir.exists()) {
                return false;
            }

            return basedir.exists("ToEE1.dat") || basedir.exists("data");
        }

        QString chooseBasedir(const QDir &basedir) {
            return QFileDialog::getExistingDirectory(NULL,
                                              "Please choose your ToEE installation directory.",
                                              basedir.absolutePath());
        }

        bool loadVirtualFileSystems() {
            QSettings settings;

            QDir defaultBasedir = getDefaultBasePath();
            QString basepath = settings.value("basepath", defaultBasedir.absolutePath()).toString();

            QDir basedir = QDir(basepath);

            while (!validateBasedir(basedir)) {
                QString dir = chooseBasedir(basedir);

                if (dir.isNull()) {
                    return false;
                }

                basedir = QDir(dir);
            }

            if (basedir != defaultBasedir) {
                settings.setValue("basepath", basedir.absolutePath());
            }

            vfs->add(new TroikaArchive(basedir.filePath("ToEE1.dat"), vfs));
            vfs->add(new TroikaArchive(basedir.filePath("ToEE2.dat"), vfs));
            vfs->add(new TroikaArchive(basedir.filePath("ToEE3.dat"), vfs));
            vfs->add(new TroikaArchive(basedir.filePath("ToEE4.dat"), vfs));
            vfs->add(new TroikaArchive(basedir.filePath("modules/ToEE.dat"), vfs));
            return true;
        }
    };

    Game::Game(QObject *parent) :
        QObject(parent),
        d_ptr(new GameData)
    {
        // Set default values for org+app so QSettings uses them everywhere
        QCoreApplication::setOrganizationName("Sebastian Hartte");
        QCoreApplication::setOrganizationDomain("toee.hartte.de");
        QCoreApplication::setApplicationName("EvilTemple");

        d_ptr->vfs = new VirtualFileSystem(this);
        d_ptr->vfs->setObjectName("virtualFileSystem"); // For scripting
        d_ptr->loadVirtualFileSystems();

        d_ptr->camera = new Camera(this);
        d_ptr->camera->setObjectName("camera");

        d_ptr->cursors = new Cursors(d_ptr->vfs, this);
        d_ptr->cursors->setObjectName("cursors");

        d_ptr->materials = new Materials(d_ptr->vfs, this);
        d_ptr->materials->setObjectName("materials");

        d_ptr->zoneTemplates = new ZoneTemplates(*this, this);
        d_ptr->zoneTemplates->setObjectName("zoneTemplates");

        d_ptr->prototypes = new Prototypes(d_ptr->vfs, this);
        d_ptr->prototypes->setObjectName("prototypes");

        d_ptr->models = new Models(d_ptr->vfs, d_ptr->materials, this);
        d_ptr->models->setObjectName("models");

        d_ptr->scriptEngine = new ScriptEngine(this);
        d_ptr->scriptEngine->setObjectName("scriptEngine");

        d_ptr->campaign = NULL;
    }

    Game::~Game() {
    }

    bool Game::start() {

        return true;
    }

    VirtualFileSystem *Game::virtualFileSystem() const {
        return d_ptr->vfs;
    }

    Cursors *Game::cursors() const {
        return d_ptr->cursors;
    }

    Materials *Game::materials() const {
        return d_ptr->materials;
    }

    Camera *Game::camera() const {
        return d_ptr->camera;
    }

    ZoneTemplates *Game::zoneTemplates() const
    {
        return d_ptr->zoneTemplates;
    }

    Campaign *Game::campaign() const
    {
        return d_ptr->campaign;
    }

    Prototypes *Game::prototypes() const
    {
        return d_ptr->prototypes;
    }

    Models *Game::models() const
    {
        return d_ptr->models;
    }

    ScriptEngine *Game::scriptEngine() const
    {
        return d_ptr->scriptEngine;
    }

    void Game::newCampaign()
    {
        delete d_ptr->campaign;
        d_ptr->campaign = new Campaign(this);
        d_ptr->campaign->setObjectName("campaign"); // This is neccessary for scripting

        d_ptr->campaign->loadMap(5001);
    }

}
