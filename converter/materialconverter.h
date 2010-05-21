#ifndef MATERIALCONVERTER_H
#define MATERIALCONVERTER_H

#include <QScopedPointer>
#include <QList>
#include <QByteArray>

#include "material.h"

namespace Troika {
    class VirtualFileSystem;
}
class ZipWriter;
class QString;
class MaterialConverterData;

class HashedData {
public:
	HashedData() {}
    HashedData(const QByteArray &data);
    QByteArray md5Hash;
    QByteArray data;
};

QDataStream &operator <<(QDataStream &stream, const HashedData &hashedData);

/**
  Converts a single material from the ToEE MDF format to CgFx.
  Includes conversion and possible compression of the texture.
  */
class MaterialConverter
{
public:
    MaterialConverter(Troika::VirtualFileSystem *vfs);
    ~MaterialConverter();

    bool convert(const Troika::Material *material);

    /**
      Returns the textures to be embedded into the file enclosing the material.
      */
    const QMap<QString,HashedData> &textures();

    /**
      Returns the material scripts generated by this converter.
      */
    const QMap<QString,HashedData> &materialScripts();

	void setExternal(bool external);
private:
    QScopedPointer<MaterialConverterData> d_ptr;
};

#endif // MATERIALCONVERTER_H
