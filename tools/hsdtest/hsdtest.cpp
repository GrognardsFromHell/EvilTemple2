
#include <QtCore/QCoreApplication>
#include <QImage>

#include "zonetemplates.h"
#include "zonetemplate.h"
#include "zonetemplatereader.h"
#include "virtualfilesystem.h"
#include "troikaarchive.h"

using namespace Troika;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    VirtualFileSystem vfs;
    vfs.loadDefaultArchives("C:/Games/ToEE/");

    Prototypes prototypes(&vfs);

    ZoneTemplates zoneTemplates(&vfs, &prototypes);

    ZoneTemplate *zoneTemplate = zoneTemplates.load(5068);

    if (!zoneTemplate) {
        qFatal("Unable to load imeryds run.");
    }

    foreach (const TileSector &tileSector, zoneTemplate->tileSectors()) {
        QImage waterTiles(192, 192,  QImage::Format_ARGB32);

        for (int x = 0; x < 192; x++) {
            for (int y = 0; y < 192; y++) {
                uchar d = tileSector.negativeHeight[x][y];

                waterTiles.setPixel(QPoint(x, y), qRgb(d, d, d));
            }
        }

        waterTiles.save(QString("%1-%2.png").arg(tileSector.x).arg(tileSector.y));
    }

    delete zoneTemplate;

}
