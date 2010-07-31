
#include "tga.h"

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
#include <QtCore/QDataStream>
#endif

namespace EvilTemple {

bool TargaImage::load()
{
    if (mData.size() < TargaHeaderSize) {
        mError = QString("Targa file has no header.");
        return false;
    }

    if (mData.size() < TargaHeaderSize + mData[0]) {
        mError = QString("Targa file is too short for image id field.");
        return false;
    }

    if (mData[1] != 0) {
        mError = QString("Unsupported image format %1.").arg(mData[0]);
        return false;
    }

    if (mData[2] != 2) {
        mError = QString("Unsupported image encoding %1. Only raw RGB is supported.").arg(mData[2]);
        return false;
    }

    // Retrieve the image viewport
    qint16 width, height;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    QDataStream stream(mData);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.skipRawData(12);
    stream >> width >> height;
#else
    width = *reinterpret_cast<const qint16*>(mData.data() + 12);
    height = *reinterpret_cast<const qint16*>(mData.data() + 14);
#endif

    if (width < 0 || height < 0) {
        mError = QString("Invalid image dimensions: %1,%2.").arg(width).arg(height);
        return false;
    }

    mWidth = width;
    mHeight = height;

    // We don't support interleaved images
    int descriptor = mData[17];
    if ((descriptor & 0xC0) != 0) {
        mError = QString("Interleaved targa images are not supported.");
        return false;
    }

    // Determine the OpenGL format
    int bitsPerPixel = mData[16];

    if (bitsPerPixel == 32) {
        mFormat = GL_BGRA;
        mInternalFormat = GL_RGBA;
    } else if (bitsPerPixel == 24) {
        mFormat = GL_BGR;
        mInternalFormat = GL_RGB;
    } else {
        mError = QString("Unsupported bits per pixel %d.").arg(bitsPerPixel);
        return false;
    }

    return true;
}

}
