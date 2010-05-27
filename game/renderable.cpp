
#include "renderable.h"
#include "scenenode.h"

namespace EvilTemple {

Renderable::Renderable()
    : mParentNode(NULL)
{
}

Renderable::~Renderable()
{
}

void Renderable::elapseTime(float secondsElapsed)
{
}

const Matrix4 &Renderable::worldTransform() const
{
    Q_ASSERT(mParentNode);
    return mParentNode->fullTransform();
}

};
