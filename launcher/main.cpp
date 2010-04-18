
#include <QtGui>

#include "game.h"
#include "datafileengine.h"
#include "ui/mainwindow.h"

using namespace EvilTemple;

int main(int argc, char *argv[])
{       
    QApplication a(argc, argv);

    DataFileEngineHandler dataFileHandler("data");

    Game game;
    if (!game.start()) {
        return -1;
    }

    MainWindow mainWindow(game);
    mainWindow.readSettings();
    mainWindow.showFromSettings();

    return a.exec();
}
