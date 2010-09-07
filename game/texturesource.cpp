
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QCryptographicHash>
#include <QtGui/QImage>

#include "texture.h"
#include "texturesource.h"

namespace EvilTemple {

    SharedTexture FileTextureSource::loadTexture(const QString &name)
    {
        QByteArray hash = QCryptographicHash::hash(QDir::toNativeSeparators(name).toLower().toUtf8(),
                                                   QCryptographicHash::Md5);
        Md5Hash filenameHash = *reinterpret_cast<const Md5Hash*>(hash.constData());

        // Check if there already is a texture in the cache
        SharedTexture texture = GlobalTextureCache::instance()->get(filenameHash);

        if (!texture) {
            QFile file(name);
            if (!file.open(QIODevice::ReadOnly)) {
                qWarning("Unable to open texture %s.", qPrintable(name));
            } else {
                QByteArray textureData = file.readAll();
                texture = SharedTexture(new Texture);
                if (name.toLower().endsWith(".tga")) {
                    texture->loadTga(textureData);
                } else {
                    QImage image;
                    if (!image.loadFromData(textureData)) {
                        qWarning("Unable to open texture: %s (using QImage codec)", qPrintable(name));
                    } else {
                        texture->load(image);
                    }
                }
                file.close();
            }

            GlobalTextureCache::instance()->insert(filenameHash, texture);
        }

        return texture;
    }

    FileTextureSource FileTextureSource::mInstance;

};
