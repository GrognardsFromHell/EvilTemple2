
#include "scenenode.h"
#include "renderqueue.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

SceneNode::SceneNode()
        : mInteractive(true), 
        mPosition(0, 0, 0, 0), 
        mScale(1, 1, 1, 1), 
        mRotation(0, 0, 0, 1), 
        mWorldMatrixInvalid(true),
        mFullTransformInvalid(true),
        mBoundingBoxInvalid(true),
        mWorldBoundingBoxInvalid(true),
        mAnimated(false),
        mParent(NULL),
        mScene(NULL)
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
            Renderable *renderable = mAttachedObjects[i].data();
            renderQueue->addRenderable(renderable->renderCategory(), renderable);
        }

        if (addChildren) {
            for (int i = 0; i < mChildren.size(); ++i) {
                mChildren[i]->addVisibleObjects(viewFrustum, renderQueue, addChildren);
            }
        }
    }

}

void SceneNode::updateFullTransform() const
{
    if (!mParent) {
        mFullTransform = worldMatrix();
    } else {
        mFullTransform = mParent->fullTransform() * worldMatrix();
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

void SceneNode::attachObject(const SharedRenderable &sharedRenderable)
{
    // TODO: Detach object from previous owner
    sharedRenderable->setParentNode(this);
    mAttachedObjects.append(sharedRenderable);
    mBoundingBoxInvalid = true;
    mWorldBoundingBoxInvalid = true;
}

};
