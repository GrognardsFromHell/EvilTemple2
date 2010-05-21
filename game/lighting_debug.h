#ifndef LIGHTING_DEBUG_H
#define LIGHTING_DEBUG_H

#include "lighting.h"
#include "materialstate.h"

namespace EvilTemple {

class LightDebugRenderer
{
public:
    LightDebugRenderer(RenderStates &renderStates);

    bool loadMaterial();

    void render(const Light &light);

private:
    RenderStates &mRenderStates;
    MaterialState mMaterialState;
};

}

#endif // LIGHTING_DEBUG_H
