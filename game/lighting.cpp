
#include "renderstates.h"
#include "util.h"
#include "lighting.h"
#include "lighting_debug.h"
#include "scenenode.h"

namespace EvilTemple {

Vector4 Light::position() const
{
    if (mParentNode == NULL) {
        return Vector4(0, 0, 0, 1);
    } else {
        const Matrix4 &fullTransform = mParentNode->fullTransform();
        return fullTransform.column(3);
    }
}

void Light::render(RenderStates &renderStates)
{
    if (!mDebugging || !mDebugRenderer)
        return;

    mDebugRenderer->render(*this);
}

void Light::setDebugRenderer(LightDebugRenderer *debugRenderer)
{
    mDebugRenderer = debugRenderer;
}

LightDebugRenderer *Light::mDebugRenderer = NULL;

}
