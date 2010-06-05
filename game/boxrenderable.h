
#ifndef BOXRENDERABLE_H
#define BOXRENDERABLE_H

#include "materialstate.h"
#include "renderable.h"
#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class Materials;

class BoxRenderable : public Renderable {
public:
    BoxRenderable(Materials *materials);

    void render(RenderStates &renderStates);

    const Box3d &boundingBox();

private:
    SharedMaterialState mMaterial;
};

}

#endif // BOXRENDERABLE_H
