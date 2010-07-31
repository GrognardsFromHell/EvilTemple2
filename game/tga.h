#ifndef TGA_H
#define TGA_H

#include <GL/glew.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>

namespace EvilTemple {

/**
  A bare-bone loader that supports a subset of the targa image format.

  Only 24 or 32 bit uncompressed (no RLE) targa images are supported.
  */
class TargaImage {
public:
    /**
      Constructs a targa image that will read from the given byte array.
      The image will not be loaded yet. Call load for that.
      */
    TargaImage(const QByteArray &data);

    /**
      Tries to load a targa image from the given byte array.
      */
    bool load();

    /**
      The raw data of this image, in the format indicated by the format getter.
      */
    const char *data() const;

    /**
      Width of this image in pixel.
      */
    uint width() const;

    /**
      Height of this image in pixel.
      */
    uint height() const;

    /**
      OpenGL compatible description of the image's format.
      */
    GLenum format() const;

    /**
      An internal format that can be used to represent this texture.
      This will be either GL_RGB for 24-bit textures or GL_RGBA for 32-bit textures.
      */
    GLenum internalFormat() const;

    /**
      Returns a string representation of an error if an error occured.
      */
    const QString &error() const;

private:
    QString mError;

    /**
      Size of targa header without variable length image id field.
      */
    const static int TargaHeaderSize = 18;

    uint mWidth;
    uint mHeight;
    GLenum mFormat;
    GLenum mInternalFormat;
    const QByteArray &mData;
};

inline TargaImage::TargaImage(const QByteArray &data) : mData(data)
{
}

inline const char *TargaImage::data() const
{
    // Skip to the header and "image id" field (length given by first field of header)
    return mData.data() + mData[0] + TargaHeaderSize;
}

inline uint TargaImage::width() const
{
    return mWidth;
}

inline uint TargaImage::height() const
{
    return mHeight;
}

inline GLenum TargaImage::format() const
{
    return mFormat;
}

inline const QString &TargaImage::error() const
{
    return mError;
}

inline GLenum TargaImage::internalFormat() const
{
    return mInternalFormat;
}

}

#endif // TGA_H
