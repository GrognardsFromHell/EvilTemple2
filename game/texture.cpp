
#include "texture.h"

#include <QtOpenGL/QGLContext>
#include <QtCore/QWaitCondition>

#include "turbojpeg.h"
#include "tga.h"

static tjhandle turboJpeg; // TODO: Clean up this handle

namespace EvilTemple {

static uint activeTextures = 0;

uint getActiveTextures()
{
    return activeTextures;
}

Texture::Texture() : mValid(false), mHandle(0), mMinFilter(GL_LINEAR), mMagFilter(GL_LINEAR),
    mWrapModeS(GL_REPEAT), mWrapModeT(GL_REPEAT)
{
    activeTextures++;
}

Texture::~Texture()
{
    activeTextures--;

    if (mValid)
        glDeleteTextures(1, &mHandle);
}

// map from Qt's ARGB endianness-dependent format to GL's big-endian RGBA layout
static inline void qgl_byteSwapImage(QImage &image, GLenum pixel_type)
{
    const int width = image.width();
    const int height = image.height();

    if (pixel_type == GL_UNSIGNED_INT_8_8_8_8_REV
        || (pixel_type == GL_UNSIGNED_BYTE && QSysInfo::ByteOrder == QSysInfo::LittleEndian))
    {
        for (int i = 0; i < height; ++i) {
            uint *p = (uint *) image.scanLine(i);
            for (int x = 0; x < width; ++x)
                p[x] = ((p[x] << 16) & 0xff0000) | ((p[x] >> 16) & 0xff) | (p[x] & 0xff00ff00);
        }
    } else {
        for (int i = 0; i < height; ++i) {
            uint *p = (uint *) image.scanLine(i);
            for (int x = 0; x < width; ++x)
                p[x] = (p[x] << 8) | ((p[x] >> 24) & 0xff);
        }
    }
}

inline bool isPowerOfTwo(int x)
{
        return (x & (x - 1)) == 0;
}

bool Texture::load(QImage image)
{
    Q_ASSERT(isPowerOfTwo(image.width()));
    Q_ASSERT(isPowerOfTwo(image.height()));

    release();

    glGenTextures(1, &mHandle);
    mValid = true;

    GLint internalFormat = GL_RGBA;
    QImage::Format target_format = image.format();

    GLenum externalFormat;
    GLuint pixel_type;
    if (GL_EXT_bgra) {
        externalFormat = GL_BGRA;
        pixel_type = GL_UNSIGNED_BYTE;
    } else {
        externalFormat = GL_RGBA;
        pixel_type = GL_UNSIGNED_BYTE;
    }

    switch (target_format) {
        case QImage::Format_ARGB32:
            break;
        case QImage::Format_ARGB32_Premultiplied:
            image = image.convertToFormat(target_format = QImage::Format_ARGB32);
            break;
        case QImage::Format_RGB16:
            pixel_type = GL_UNSIGNED_SHORT_5_6_5;
            externalFormat = GL_RGB;
            internalFormat = GL_RGB;
            break;
        case QImage::Format_RGB32:
            break;
        default:
            if (image.hasAlphaChannel()) {
                image = image.convertToFormat(QImage::Format_ARGB32);
            } else {
                image = image.convertToFormat(QImage::Format_RGB32);
            }
        }

    if (externalFormat == GL_RGBA) {
        // The only case where we end up with a depth different from
        // 32 in the switch above is for the RGB16 case, where we set
        // the format to GL_RGB
        Q_ASSERT(image.depth() == 32);
        qgl_byteSwapImage(image, pixel_type);
    }

     if (image.isDetached()) {
         int ipl = image.bytesPerLine() / 4;
         int h = image.height();
         for (int y=0; y<h/2; ++y) {
             int *a = (int *) image.scanLine(y);
             int *b = (int *) image.scanLine(h - y - 1);
             for (int x=0; x<ipl; ++x)
                 qSwap(a[x], b[x]);
         }
     } else {
         // Create a new image and copy across.  If we use the
         // above in-place code then a full copy of the image is
         // made before the lines are swapped, which processes the
         // data twice.  This version should only do it once.
         image = image.mirrored();
     }

    glBindTexture(GL_TEXTURE_2D, mHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mMinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapModeS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapModeT);

    const QImage &constRef = image; // to avoid detach in bits()...
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.width(), image.height(), 0, externalFormat,
                 pixel_type, constRef.bits());
    glBindTexture(GL_TEXTURE_2D, 0);

    mWidth = image.width();
    mHeight = image.height();

    return true;
}

bool Texture::loadTga(const QByteArray &tgaImage)
{
    TargaImage image(tgaImage);

    if (!image.load()) {
        qWarning("TGA loading error: %s", image.error());
        return false;
    }

    if (isValid()) {
            glDeleteTextures(1, &mHandle);
            mValid = false;
    }

    glGenTextures(1, &mHandle);
    mValid = true;

    glBindTexture(GL_TEXTURE_2D, mHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mMinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mMagFilter);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapModeS);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapModeT);
    glTexImage2D(GL_TEXTURE_2D, 0, image.internalFormat(), image.width(), image.height(), 0, image.format(),
                 GL_UNSIGNED_BYTE, image.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    mWidth = image.width();
    mHeight = image.height();

    return true;
}

bool Texture::loadJpeg(const QByteArray &jpegImage)
{
    if (!turboJpeg) {
        turboJpeg = tjInitDecompress();
    }

    int width, height;

    if (tjDecompressHeader(turboJpeg, (uchar*)jpegImage.data(), jpegImage.size(), &width, &height)) {
        return false;
    }

    Q_ASSERT(isPowerOfTwo(width));
    Q_ASSERT(isPowerOfTwo(height));

    QByteArray decompressedImage(width * height * 3, Qt::Uninitialized);

    if (tjDecompress(turboJpeg, (uchar*)jpegImage.data(), jpegImage.size(), (uchar*)decompressedImage.data(),
                     width, width * 3, height, 3, TJ_BOTTOMUP))
    {
        return false;
    }

    if (isValid()) {
            glDeleteTextures(1, &mHandle);
            mValid = false;
    }

    glGenTextures(1, &mHandle);
    mValid = true;

    glBindTexture(GL_TEXTURE_2D, mHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mMinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapModeS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapModeT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, decompressedImage.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    mWidth = width;
    mHeight = height;

    return true;

}

void Texture::release()
{
    if (isValid()) {
            glDeleteTextures(1, &mHandle);
            mValid = false;
    }
}

class GlobalTextureCacheCleanupThread : public QThread
{
public:
    const static uint TexturePruneInterval = 2500; // Milliseconds

    GlobalTextureCacheCleanupThread(GlobalTextureCache &cache)
        : mCache(cache), mCancel(false)
    {
    }

    void cancel()
    {
        mCancel = true;
    }

protected:
    void run()
    {
        QMutexLocker locker(&mCache.mCleanupMutex);
        while (!mCache.mWaitCondition.wait(&mCache.mCleanupMutex, TexturePruneInterval) && !mCancel) {
            pruneTextures();
        }
    }

private:

    void pruneTextures()
    {
        GlobalTextureCache::iterator it = mCache.mTextures.begin();

        while (it != mCache.mTextures.end()) {
            if (it.value().isNull()) {
                it = mCache.mTextures.erase(it);
            } else {
                it++;
            }
        }
    }

    volatile bool mCancel;

    GlobalTextureCache &mCache;
};

GlobalTextureCache::GlobalTextureCache() : mThread(new GlobalTextureCacheCleanupThread(*this))
{
    mThread->start();
}

GlobalTextureCache::~GlobalTextureCache()
{
    qDebug("Shutting down the global texture cache cleanup thread.");
    mThread->cancel();
    mWaitCondition.wakeAll();
    mThread->wait();
    delete mThread;
}

SharedTexture GlobalTextureCache::get(const Md5Hash &hash)
{
    QMutexLocker locker(&mCleanupMutex);

    iterator it = mTextures.find(hash);

    if (it != mTextures.end()) {
        return SharedTexture(it.value());
    } else {
        return SharedTexture(0);
    }
}

void GlobalTextureCache::insert(const Md5Hash &hash, const SharedTexture &texture)
{
    QMutexLocker locker(&mCleanupMutex);

    mTextures[hash] = QWeakPointer<Texture>(texture);
}

void GlobalTextureCache::start()
{
    Q_ASSERT(!mInstance);
    mInstance = new GlobalTextureCache;
}

void GlobalTextureCache::stop()
{
    delete mInstance;
}

GlobalTextureCache *GlobalTextureCache::mInstance = NULL;

}
