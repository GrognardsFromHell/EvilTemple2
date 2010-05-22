
#include <QtCore/QFile>
#include <QtCore/QCryptographicHash>

#include "texture.h"
#include "texturesource.h"

namespace EvilTemple {

SharedTexture FileTextureSource::loadTexture(const QString &name)
{
	bool ok;
			
	QByteArray hash = QCryptographicHash::hash(QDir::toNativeSeparators(name).toLower().toUtf8(), QCryptographicHash::Md5);
	Md5Hash filenameHash = *reinterpret_cast<const Md5Hash*>(hash.constData());

    // Check if there already is a texture in the cache
    SharedTexture texture = GlobalTextureCache::instance().get(filenameHash);

    if (!texture) {
		QFile file(name);
		if (!file.open(QIODevice::ReadOnly)) {
			qWarning("Unable to open texture %s.", qPrintable(name));
		} else {
			QByteArray textureData = file.readAll();
			texture = SharedTexture(new Texture);
			texture->loadTga(textureData);
			file.close();
		}

        GlobalTextureCache::instance().insert(filenameHash, texture);
    }

	return texture;
}

FileTextureSource FileTextureSource::mInstance;

};
