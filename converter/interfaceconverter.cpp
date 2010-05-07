
#include <QStringList>
#include <QString>
#include <QBuffer>
#include <QImage>
#include <QPainter>
#include "zipwriter.h"
#include "virtualfilesystem.h"

#include "interfaceconverter.h"

class InterfaceConverterData {
public:

    ZipWriter *zip;
    Troika::VirtualFileSystem *vfs;

    QStringList excludePaths; // Paths to exclude in the texture conversion step

    InterfaceConverterData(ZipWriter *_zip, Troika::VirtualFileSystem *_vfs) : zip(_zip), vfs(_vfs)
    {
    }

    static const int tileWidth = 256;
    static const int tileHeight = tileWidth;

    void convertImage(const QString &baseName, int width, int height)
    {
        QImage image(width, height, QImage::Format_ARGB32);

        int xTiles = (width + 255) / 256; // Round up
        int yTiles = (height + 255) / 256; // Also round up

        int destY = height; // We're counting down here.

        for (int y = 0; y < yTiles; ++y) {

            int destX = 0;
            int lastTileHeight;
            for (int x = 0; x < xTiles; ++x) {
                QString imagePath = QString("%1_%2_%3.tga").arg(baseName).arg(x).arg(y);
                QByteArray tgaData = vfs->openFile(imagePath);
                QImage tile;

                if (!tile.loadFromData(tgaData, "tga")) {
                    qWarning("Unable to load tile %s (TGA) of combined image.", qPrintable(imagePath));
                    continue;
                }

                excludePaths.append(imagePath); // Make sure it doesnt get converted twice

                for (int sy = 0; sy < tile.height(); ++sy) {
                    uchar *destScanline = image.scanLine(destY - tile.height() + sy);
                    destScanline += destX * sizeof(QRgb);
                    uchar *srcScanline = tile.scanLine(sy);

                    qMemCopy(destScanline, srcScanline, sizeof(QRgb) * tile.width());
                }

                destX += tileWidth;
                lastTileHeight = tile.height();
            }

            destY -= tileHeight; // We're counting down
        }

        QByteArray pngData;
        QBuffer pngBuffer(&pngData);

        if (!image.save(&pngBuffer, "png")) {
            qWarning("Unable to save image %s.img (PNG).", qPrintable(baseName));
        } else {
            pngBuffer.close();

            zip->addFile(baseName + ".png", pngData, 0);
        }
    }

    void convertImages()
    {
        QStringList images = vfs->listAllFiles("*.img");

        foreach (const QString &imageFile, images) {

            QDataStream imgFile(vfs->openFile(imageFile));
            imgFile.setByteOrder(QDataStream::LittleEndian);

            quint16 width, height;
            imgFile >> width >> height;

            // Strip the .img suffix for the base name
            QString baseName = imageFile.left(imageFile.length() - 4);

            convertImage(baseName, width, height);
        }
    }

    void convertTextures()
    {
        QStringList images = vfs->listAllFiles("*.tga");

        foreach (const QString &imagePath, images) {

            if (QDir::toNativeSeparators(imagePath).startsWith(QDir::toNativeSeparators("art/meshes/"),
                                                               Qt::CaseInsensitive)) {
                continue;
            }

            if (excludePaths.contains(imagePath)) {
                qDebug("Skipping %s because it is excluded.", qPrintable(imagePath));
                continue;
            }

            QByteArray tgaData = vfs->openFile(imagePath);
            QImage image;

            if (!image.loadFromData(tgaData, "tga")) {
                qWarning("Unable to load image %s (TGA).", qPrintable(imagePath));
                continue;
            }

            QString pngPath = imagePath;
            pngPath.replace(QRegExp("\\.tga$", Qt::CaseInsensitive), ".png");

            QByteArray pngData;
            QBuffer pngBuffer(&pngData);

            if (!image.save(&pngBuffer, "png")) {
                qWarning("Unable to save image %s (PNG).", qPrintable(imagePath));
                continue;
            }

            pngBuffer.close();

            zip->addFile(pngPath, pngData, 0);
        }
    }

};

InterfaceConverter::InterfaceConverter(ZipWriter *zip, Troika::VirtualFileSystem *vfs)
    : d_ptr(new InterfaceConverterData(zip, vfs))
{
}

InterfaceConverter::~InterfaceConverter()
{
}

bool InterfaceConverter::convert()
{
    d_ptr->convertImages();
    d_ptr->convertTextures();
    return true;
}
