
#include <QImage>

#include "mapconverter.h"
#include "zonebackgroundmap.h"
#include "squish.h"
#include "jpeglib.h"

template <int T> bool checkBlack(unsigned char *pixels, int width, int height) {
    int totalPixels = width * height;
    uchar *current = pixels;
    uchar *end = pixels + totalPixels;

    while (current < end) {
        if (*(current++) > 3)
            return false;
        if (*(current++) > 3)
            return false;
        if (*(current++) > 3)
            return false;
        current += T;
    }

    return true;
}

class MapConverterData
{
public:
    MapConverterData(VirtualFileSystem *vfs, ZipWriter *zip) : mVfs(vfs), mZip(zip)
    {
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);
    }

    ~MapConverterData()
    {
        jpeg_destroy_decompress(&cinfo);
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    void decompressJpeg(const QByteArray &input, QByteArray &output, int &width, int &height, int &components) {
        jpeg_mem_src(&cinfo, (unsigned char*)input.data(), input.size());

        jpeg_read_header(&cinfo, TRUE);

        jpeg_start_decompress(&cinfo);

        int scanlineSize = cinfo.output_components * cinfo.output_width;
        output.resize(scanlineSize * cinfo.output_height);

        uchar** scanlines = new uchar*[cinfo.output_height];
        uchar *buffer = (uchar*)output.data();
        for (int i = 0; i < cinfo.output_height; ++i) {
            scanlines[i] = buffer;
            buffer += scanlineSize;
        }

        uchar** currentScanline = scanlines;
        while (cinfo.output_scanline < cinfo.output_height) {
            int read = jpeg_read_scanlines(&cinfo, currentScanline, cinfo.output_height - cinfo.output_scanline);
            currentScanline += read;
        }

        delete [] scanlines;

        width = cinfo.output_width;
        height = cinfo.output_height;
        components = cinfo.output_components;

        jpeg_finish_decompress(&cinfo);
    }

    /**
      Converts a background map and returns the new entry point file for it.
      */
    QString convertGroundMap(const ZoneBackgroundMap *background) {

        QString directory = background->directory().toLower();

        // Check if the map has already been converted
        if (convertedGroundMaps.contains(directory)) {
            return convertedGroundMaps[directory];
        }

        QString newFolder = "backgroundMaps/";

        // Get last directory name
        QStringList parts = directory.split('/', QString::SkipEmptyParts);
        QString dirname = parts.last();
        // Remove superflous information from the dirname
        dirname.replace(QRegExp("map\\d*\\-?\\d+\\-?", Qt::CaseInsensitive), "");
        newFolder = newFolder + dirname + "/";

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

            mZip->addFile(QString("%1%2-%3.jpg").arg(newFolder).arg(y).arg(x), tileContent, 0);
        }

        convertedGroundMaps[directory] = newFolder;

        return newFolder;
    }

    bool isTileBlack(const QImage &image)
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

private:
    VirtualFileSystem *mVfs;
    ZipWriter *mZip;
    QHash<QString,QString> convertedGroundMaps;
};

MapConverter::MapConverter(VirtualFileSystem *vfs, ZipWriter *writer) : d_ptr(new MapConverterData(vfs, writer))
{
}

MapConverter::~MapConverter()
{
}

bool MapConverter::convert(const ZoneTemplate *zoneTemplate)
{

    if (zoneTemplate->dayBackground()) {
        d_ptr->convertGroundMap(zoneTemplate->dayBackground());
    }

    if (zoneTemplate->nightBackground()) {
        d_ptr->convertGroundMap(zoneTemplate->nightBackground());
    }

    return true;
}
