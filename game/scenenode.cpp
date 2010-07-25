
#include "scenenode.h"
#include "renderqueue.h"
#include "scene.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

SceneNode::SceneNode(Scene *scene)
        : QObject(scene), mInteractive(false),
        mPosition(0, 0, 0, 0),
        mScale(1, 1, 1, 1),
        mRotation(0, 0, 0, 1),
        mWorldMatrixInvalid(true),
        mFullTransformInvalid(true),
        mBoundingBoxInvalid(true),
        mWorldBoundingBoxInvalid(true),
        mAnimated(false),
        mParentNode(NULL),
        mScene(scene)
{
}

SceneNode::~SceneNode()
{
}

void SceneNode::elapseTime(float elapsedSeconds)
{
    for (int i = 0; i < mAttachedObjects.size(); ++i) {
        mAttachedObjects[i]->elapseTime(elapsedSeconds);
    }
}

IntersectionResult SceneNode::intersect(const Ray3d &ray) const
{
    Q_UNUSED(ray);
    IntersectionResult result;
    result.distance = std::numeric_limits<float>::infinity();
    result.intersects = false;
    return result;
}

void SceneNode::addVisibleObjects(const Frustum &viewFrustum, RenderQueue *renderQueue, bool addChildren)
{

    if (viewFrustum.isVisible(worldBoundingBox())) {
        for (int i = 0; i < mAttachedObjects.size(); ++i) {
            Renderable *renderable = mAttachedObjects[i];
            renderQueue->addRenderable(renderable->renderCategory(), renderable);
        }

        if (addChildren) {
            for (int i = 0; i < mChildNodes.size(); ++i) {
                mChildNodes[i]->addVisibleObjects(viewFrustum, renderQueue, addChildren);
            }
        }
    }

}

void SceneNode::updateFullTransform() const
{
    if (!mParentNode) {
        mFullTransform = worldMatrix();
    } else {
        mFullTransform = mParentNode->fullTransform() * worldMatrix();
    }
    mFullTransformInvalid = false;
}

void SceneNode::updateBoundingBox() const
{
    if (mAttachedObjects.size() == 0) {
        mBoundingBox = Box3d(); // Null box
    } else {
        mBoundingBox = mAttachedObjects[0]->boundingBox();

        for (int i = 1; i < mAttachedObjects.size(); ++i) {
            const Box3d &bounds = mAttachedObjects[i]->boundingBox();
            if (!bounds.isNull())
                mBoundingBox.merge(bounds);
        }
    }

    mBoundingBoxInvalid = false;
}

void SceneNode::attachObject(Renderable *renderable)
{
    if (!renderable) {
        qWarning("Trying to attach a null-renderable to a scene node.");
        return;
    }

    // TODO: Fix this mess
    if (!renderable->parent()) {
        renderable->setParent(mScene);
    }

    if (!mAttachedObjects.contains(renderable)) {
        // TODO: Detach object from previous owner
        renderable->setParentNode(this);
        mAttachedObjects.append(renderable);
        mBoundingBoxInvalid = true;
        mWorldBoundingBoxInvalid = true;
    }
}

void SceneNode::detachObject(Renderable *renderable)
{
    mAttachedObjects.removeOne(renderable);
}

};
