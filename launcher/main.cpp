
#include <QtGui>

#include "game.h"
#include "datafileengine.h"
#include "mainwindow.h"
#include "scriptengine.h"

using namespace EvilTemple;

int main(int argc, char *argv[])
{       
    QApplication a(argc, argv);

    // Set default values for org+app so QSettings uses them everywhere
    QCoreApplication::setOrganizationName("Sebastian Hartte");
    QCoreApplication::setOrganizationDomain("toee.hartte.de");
    QCoreApplication::setApplicationName("EvilTemple");

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
