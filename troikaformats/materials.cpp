
#include <GL/glew.h>

#include <QTime>
#include <QMatrix4x4>
#include <QColor>

#include <QtOpenGL>

#include "materials.h"
#include "material.h"
#include "virtualfilesystem.h"
#include "util.h"

namespace Troika {

    /*
      Private implementation details class for the materials class.
     */
    typedef QHash< QString, QWeakPointer<Material> > MaterialCache;

    class MaterialsData {
    public:
        VirtualFileSystem *vfs;
        MaterialCache materials;
    };

    Materials::Materials(VirtualFileSystem *vfs, QObject *owner) :
            QObject(owner),
            d_ptr(new MaterialsData)
    {
        d_ptr->vfs = vfs;
    }

    Materials::~Materials() {

    }

    QSharedPointer<Material> Materials::loadFromFile(const QString &filename)
    {
        // See if the same material has already been loaded
        MaterialCache::iterator it = d_ptr->materials.find(filename);
        if (it != d_ptr->materials.end())
        {
            QSharedPointer<Material> material(*it);

            if (material)
            {
                return material;
            }
        }

        QSharedPointer<Material> material(Material::create(d_ptr->vfs, filename));

        // Store the material under it's name in the map
        d_ptr->materials.insert(material->name(), QWeakPointer<Material>(material));

        return material;
    }

}
