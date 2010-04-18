
#include <QThread>
#include <QApplication>


#include "mainwindow.h"

#include "audioengine.h"
#include "isound.h"
#include "isoundhandle.h"
#include "isoundsource.h"
#include "mp3reader.h"
#include "wavereader.h"

using namespace EvilTemple;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AudioEngine engine;

    foreach (QString device, engine.devices()) {
        qDebug("Device: %s", qPrintable(device));
    }

    if (!engine.open()) {
        return -1;
    }

    MainWindow mainWindow(&engine);

    mainWindow.show();

    return a.exec();
}

