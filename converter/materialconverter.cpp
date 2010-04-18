
#include <QString>

#include "virtualfilesystem.h"
#include "material.h"

#include "materialconverter.h"

using namespace Troika;

class MaterialConverterData
{
private:
    QString mFilename;
    VirtualFileSystem *mVfs;
    ZipWriter *mOutput;
public:
    MaterialConverterData(VirtualFileSystem *vfs, ZipWriter *output, const QString &filename)
        : mFilename(filename), mOutput(output), mVfs(vfs) {
    }

    bool convert() {
        QScopedPointer<Material> material(Material::create(mVfs, mFilename));

        return true;
    }

};

MaterialConverter::MaterialConverter(VirtualFileSystem *vfs, ZipWriter *output, const QString &filename)
    : d_ptr(new MaterialConverterData(vfs, output, filename))
{
}

MaterialConverter::~MaterialConverter()
{
}

bool MaterialConverter::convert()
{
    return d_ptr->convert();
}
