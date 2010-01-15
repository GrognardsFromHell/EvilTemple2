
#include <iostream>
#include <QtGui>

#include "ui/mainwindow.h"
#include "ui/cursors.h"
#include "io/troikaarchive.h"
#include "io/basepathfinder.h"
#include "io/skmreader.h"
#include "model.h"
#include "game.h"

// Import the NS if it's defined
#if !defined(EVILTEMPLE_NO_NS)
using namespace EvilTemple;
#endif

int main(int argc, char *argv[])
{       
    QApplication a(argc, argv);

    Game game;
    if (!game.start()) {
        return -1;
    }

    const QCursor &cursor = game.cursors()->get(Cursors::Normal);
    MainWindow mainWindow(game);
    mainWindow.setCursor(cursor);
    mainWindow.readSettings();
    mainWindow.showFromSettings();

    game.newCampaign();

    return a.exec();
}

Q_IMPORT_PLUGIN(tga)
