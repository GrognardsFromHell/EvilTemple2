/*
 * TgaPlugin.cpp
 *
 *  Created on: 16.12.2009
 *      Author: Sebastian
 */

#include <QDataStream>
#include <QImage>
#include <cstring>

#include "io/tgaplugin.h"

namespace EvilTemple {

TgaPlugin::TgaPlugin() {
}

TgaPlugin::~TgaPlugin() {
}

QImageIOPlugin::Capabilities TgaPlugin::capabilities(QIODevice *device,
		const QByteArray &format) const {
	Q_UNUSED(device);

	if (format == QString("tga")) {
		return CanRead;
	} else {
		return 0;
	}
}

QImageIOHandler *TgaPlugin::create(QIODevice *device, const QByteArray &format) const {
	TgaHandler *handler = new TgaHandler;
	handler->setDevice(device);
	handler->setFormat(format);
	return handler;
}

QStringList TgaPlugin::keys() const {
	QStringList formats;
	formats.append("tga");
	return formats;
}

/*
 * Low level TGA reading.
 */
struct TgaHeader {
	quint8 idLength;
	quint8 mapType;
	quint8 imageType;
	quint16 firstMapEntry;
	quint16 mapLength;
	quint8 mapEntryDepth;
	quint16 x;
	quint16 y;
	quint16 width;
	quint16 height;
	quint8 depth;
	quint8 alpha;
	bool fromTop; /* Derived from alpha bit */
	bool fromRight; /* Derived from alpha bit */
};

TgaHandler::TgaHandler() {
}

bool TgaHandler::canRead() const {
	return true;
}

inline void readScanline(QDataStream &stream, QImage &image, qint32 y, int scanlineLength, TgaHeader &header) {
    uchar* scanline = image.scanLine(y);
    stream.readRawData(reinterpret_cast<char*>(scanline), scanlineLength);
    if (header.depth == 24) {
        for (int i = 0; i < scanlineLength; i += 3)
            std::swap(scanline[i], scanline[i+2]);
    }
}

bool TgaHandler::read(QImage *image) {
	TgaHeader header;
	QDataStream stream(device());
        stream.setByteOrder(QDataStream::LittleEndian);

        stream >> header.idLength >> header.mapType >> header.imageType;
        stream >> header.firstMapEntry >> header.mapLength >> header.mapEntryDepth;
        stream >> header.x >> header.y >> header.width >> header.height >> header.depth >> header.alpha;

        // Post-process alpha quint
	header.fromTop = (header.alpha & 0x10) == 0x10;
	header.fromRight = (header.alpha & 0x20) == 0x20;
	header.alpha &= 0xf; // Only the lower 4 bits are the number of alpha bits

        Q_ASSERT(header.depth == 32 || header.depth == 24); // Only reads 24/32 bit TGAs
        Q_ASSERT(!header.fromRight); // Can't handle from right...

        QImage::Format format = (header.depth == 24) ? QImage::Format_RGB888 : QImage::Format_ARGB32;

        QImage localImage(QSize(header.width, header.height), format);

        // Start reading scanlines from either top->bottm or bottom->top (default)
        const int scanlineLength = header.width * header.depth / 8;
        if (header.fromTop) {
            for (quint16 y = 0; y < header.height; ++y)
                readScanline(stream, localImage, y, scanlineLength, header);
        } else {
            for (qint32 y = header.height - 1; y >= 0; --y)
                readScanline(stream, localImage, y, scanlineLength, header);
        }

        *image = localImage;

	return true;
}

}

QT_PREPEND_NAMESPACE(QObject) *qt_plugin_instance_tga() Q_PLUGIN_INSTANCE(EvilTemple::TgaPlugin)
