#ifndef MATERIALCONVERTER_H
#define MATERIALCONVERTER_H

#include <QScopedPointer>

namespace Troika {
    class VirtualFileSystem;
}
class ZipWriter;
class QString;
class MaterialConverterData;

/**
  Converts a single material from the ToEE MDF format to CgFx.
  Includes conversion and possible compression of the texture.
  */
class MaterialConverter
{
public:
    MaterialConverter(Troika::VirtualFileSystem *vfs, ZipWriter *output, const QString &filename);
    ~MaterialConverter();

    bool convert();
private:
    QScopedPointer<MaterialConverterData> d_ptr;
};

#endif // MATERIALCONVERTER_H
