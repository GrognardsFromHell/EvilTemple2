
#include "renderstates.h"
#include "util.h"
#include "lighting.h"
#include "lighting_debug.h"
#include "scenenode.h"

namespace EvilTemple {

Vector4 Light::position() const
{
    if (mParentNode == NULL) {
        qDebug("Querying light position, although light has no parent node.");
        return Vector4(0, 0, 0, 1);
    } else {
        Vector4 pos(0,0,0,1);

        const Matrix4 &fullTransform = mParentNode->fullTransform();

        return fullTransform * pos;
    }
}

void Light::render(RenderStates &renderStates, MaterialState *overrideMaterial = NULL)
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
