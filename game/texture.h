
#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>

#include <QtCore/QSharedPointer>
#include <QtGui/QImage>

namespace EvilTemple {

class Texture {
public:
	Texture();
	~Texture();

	void bind();
	void unbind();

	bool isValid() const;

        bool load(QImage image);

        void setMinFilter(GLenum minFilter);
        void setMagFilter(GLenum magFilter);
        void setWrapModeS(GLenum wrapMode);
        void setWrapModeT(GLenum wrapMode);

        /**
          An optimized loading method for Targa textures.
          This assumes the texture is either 24-bit or 32-bit.
          */
        bool loadTga(const QByteArray &tgaImage);

        /**
          An optimized loading method for JPEG images.
          */
        bool loadJpeg(const QByteArray &jpegImage);

        /**
          Releases resources held by this texture.
          */
        void release();
private:
	GLuint mHandle;
	bool mValid;
        GLenum mMinFilter;
        GLenum mMagFilter;
        GLenum mWrapModeS;
        GLenum mWrapModeT;
};

inline void Texture::setMinFilter(GLenum minFilter)
{
    mMinFilter = minFilter;
}

inline void Texture::setMagFilter(GLenum magFilter)
{
    mMagFilter = magFilter;
}

inline void Texture::setWrapModeS(GLenum wrapMode)
{
    mWrapModeS = wrapMode;
}

inline void Texture::setWrapModeT(GLenum wrapMode)
{
    mWrapModeT = wrapMode;
}

inline void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, mHandle);
}

inline bool Texture::isValid() const
{
	return mHandle != -1;
}

// Holders of textures should use this pointer type instead of textures directly
typedef QSharedPointer<Texture> SharedTexture;

}

#endif
