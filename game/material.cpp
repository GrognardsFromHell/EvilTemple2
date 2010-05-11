#include "material.h"

namespace EvilTemple {

Material::Material(const QString &name) : mName(name), mNoBackfaceCulling(false), mNoDepthTest(false)
{
}

}
