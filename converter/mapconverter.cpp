
#include <QImage>
#include <QDir>

#include "mapconverter.h"
#include "zonebackgroundmap.h"
#include "turbojpeg.h"
#include "util.h"
#include "conversiontask.h"
#include "virtualfilesystem.h"

static tjhandle jpegHandle = 0;

template <int T> bool checkBlack(unsigned char *pixels, int width, int height) {
    int totalPixels = width * height;
    uchar *current = pixels;
    uchar *end = pixels + totalPixels * T;

    while (current < end) {
        if (*(current++) > 3)
            return false;
        if (*(current++) > 3)
            return false;
        if (*(current++) > 3)
            return false;
        current += (T - 3);
    }

    return true;
}

static bool isTileBlack(const QImage &image)
{
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixel = image.pixel(x, y);
            if (qRed(pixel) > 2 || qBlue(pixel) > 2 || qGreen(pixel) > 2) {
                return false;
            }
        }
    }

    return true;
}


    static void decompressJpeg(const QByteArray &input, QByteArray &output, int &width, int &height, int &components) {
        if (!jpegHandle) {
            jpegHandle = tjInitDecompress();
        }

        uchar *srcBuffer = (uchar*)input.data();

        if (tjDecompressHeader(jpegHandle, srcBuffer, input.size(), &width, &height)) {
            qWarning("Unable to read JPEG header.");
            return;
        }

        components = 3;
        int pitch = width * 3;
        output.resize(height * pitch);
        int flags = 0;

        if (tjDecompress(jpegHandle, srcBuffer, input.size(), (uchar*)output.data(), width, pitch, height, 3, flags)) {
            qWarning("Unable to decompress JPEG image.");
            return;
        }
    }

    /**
      Converts a background map and returns the new entry point file for it.
      */
    QString MapConverter::convertGroundMap(const ZoneBackgroundMap *background) {

        QString directory = background->directory().toLower();

        // Check if the map has already been converted
        if (convertedGroundMaps.contains(directory)) {
            return convertedGroundMaps[directory];
        }

        QString newFolder = getNewBackgroundMapFolder(background->directory());

        // Get all jpg files in the directory and load them
        QList<QPoint> tilesPresent; // Indicates which tiles are actually present in the background map

        QStringList backgroundTiles = mVfs->listFiles(background->directory(), "*.jpg");

        QByteArray decodedImage;
        int width, height, components;

        foreach (const QString &tileFilename, backgroundTiles) {
            QByteArray tileContent = mVfs->openFile(tileFilename);

            if (tileContent.isNull())
                continue;

            // Parse out x and y
            QString xAndY = tileFilename.right(12).left(8);
            int y = xAndY.left(4).toInt(0, 16);
            int x = xAndY.right(4).toInt(0, 16);

            decompressJpeg(tileContent, decodedImage, width, height, components);

            if (components == 3) {
                if (checkBlack<3>((uchar*)decodedImage.data(), width, height)) {
                    continue;
                }
            } else {
                if (checkBlack<4>((uchar*)decodedImage.data(), width, height)) {
                    continue;
                }
            }

            tilesPresent.append(QPoint(x, y));

            mWriter->addFile(QString("%1%2-%3.jpg").arg(newFolder).arg(y).arg(x), tileContent, 0);
        }

        QByteArray tileIndex;
        tileIndex.reserve(sizeof(int) + sizeof(short) * 2 * tilesPresent.size());
        QDataStream stream(&tileIndex, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);

        stream << (uint)tilesPresent.size();

        for (int i = 0; i < tilesPresent.size(); ++i) {
            const QPoint &point = tilesPresent[i];
            stream << (short)point.x() << (short)point.y();
        }

        mWriter->addFile(QString("%1index.dat").arg(newFolder), tileIndex, 9);

        convertedGroundMaps[directory] = newFolder;

        return newFolder;
    }

MapConverter::MapConverter(IConversionService *service, IFileWriter *writer)
    : mService(service), mWriter(writer), mVfs(service->virtualFileSystem())
{
}

bool MapConverter::convert(const ZoneTemplate *zoneTemplate)
{

    if (zoneTemplate->dayBackground()) {
        convertGroundMap(zoneTemplate->dayBackground());
    }

    if (zoneTemplate->nightBackground()) {
        convertGroundMap(zoneTemplate->nightBackground());
    }

    return true;
}
