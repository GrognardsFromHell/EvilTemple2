#include "material.h"
#include "io/virtualfilesystem.h"

namespace EvilTemple {

    Material::Material(const QString &name) : _name(name) {

    }

    Material::~Material() {
    }

    bool Material::hitTest(float u, float v)
    {
        Q_UNUSED(u);
        Q_UNUSED(v);

        return true;
    }

}
