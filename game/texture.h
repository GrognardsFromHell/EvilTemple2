
#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>

#include <QtCore/QSharedPointer>
#include <QtCore/QWeakPointer>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QHash>
#include <QtGui/QImage>

#include "util.h"

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

    uint width() const;
    uint height() const;

private:
    GLuint mHandle;
    bool mValid;
    GLenum mMinFilter;
    GLenum mMagFilter;
    GLenum mWrapModeS;
    GLenum mWrapModeT;
    uint mWidth;
    uint mHeight;
};

inline uint Texture::width() const
{
    return mWidth;
}

inline uint Texture::height() const
{
    return mHeight;
}

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
        return mHandle != 0;
}

// Holders of textures should use this pointer type instead of textures directly
typedef QSharedPointer<Texture> SharedTexture;


/**
  The global texture cache holds weak references to textures based on their
  Md5 hash to prevent textures from being loaded twice.

  A cleanup thread is run to prevent the cache from filling up with dead references.
  */
class GlobalTextureCacheCleanupThread;
class GlobalTextureCache {
friend class GlobalTextureCacheCleanupThread;
public:
    static GlobalTextureCache *instance() {
        return mInstance;
    }

    /**
      Tries to retrieve a texture from the texture cache by its Md5 hash.
      Returns a null pointer if no such texture exists in the cache.
      */
    SharedTexture get(const Md5Hash &hash);

    /**
      Inserts a texture into the cache.
      */
    void insert(const Md5Hash &hash, const SharedTexture &texture);

    /**
      Starts the global texture cache if it's not started yet.
      */
    static void start();

    /**
      Stops the global texture cache and clears existing data.
      */
    static void stop();

private:
    GlobalTextureCache();
    ~GlobalTextureCache();

    typedef QWeakPointer<Texture> WeakTexture;
    typedef QHash<Md5Hash, WeakTexture> CacheContainer;
    typedef CacheContainer::iterator iterator;

    CacheContainer mTextures;

    QMutex mCleanupMutex;
    QWaitCondition mWaitCondition;
    GlobalTextureCacheCleanupThread *mThread;
    static GlobalTextureCache *mInstance;
};

uint getActiveTextures();

}

#endif
