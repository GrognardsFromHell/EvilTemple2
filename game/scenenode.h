#ifndef SCENENODE_H
#define SCENENODE_H

#include <QtCore/QSharedPointer>

#include <gamemath.h>
using namespace GameMath;

#include "modelfile.h"
#include "modelinstance.h"
#include "renderable.h"
#include "renderqueue.h"

namespace EvilTemple {

class RenderStates;
class Scene;

class SceneNode
{
public:
    SceneNode();
    virtual ~SceneNode();

    const Vector4 &position() const;
    const Quaternion &rotation() const;
    const Vector4 &scale() const;
    bool isInteractive() const;
    const Matrix4 &worldMatrix() const;
    bool isAnimated() const;
    Scene *scene() const;

    void setPosition(const Vector4 &position);
    void setRotation(const Quaternion &rotation);
    void setScale(const Vector4 &scale);
    void setInteractive(bool interactive);
    void setAnimated(bool animated);
    void setScene(Scene *scene);

    const Box3d &worldBoundingBox() const;

    const Box3d &boundingBox() const;

    /**
    Tests this node for intersection.
      */
    virtual IntersectionResult intersect(const Ray3d &ray) const;

    /**
      Elapses time for this node. Only call if isAnimated() == true
      */
    virtual void elapseTime(float elapsedSeconds);

    /**
      Add all visible objects in this scene node to the given render queue and if requested,
      call this method on child nodes of this node recursively.
      */
    void addVisibleObjects(const Frustum &viewFrustum, RenderQueue *renderQueue, bool addChildren = true);
    
    /**
     Gets the full transformation matrix derived from this node's and its parent's transformations.
     */
    const Matrix4 &fullTransform() const;

    void attachObject(const SharedRenderable &sharedRenderable);
    void detachObject(const Renderable *renderable);

    const QList<SharedRenderable> &attachedObjects() const;

private:
    void updateFullTransform() const;
    void updateBoundingBox() const;

    Vector4 mScale;
    Quaternion mRotation;
    Vector4 mPosition;
    bool mInteractive;
    bool mAnimated; // requires time events
    Scene *mScene; // The scene that contains this node
    QList<SceneNode*> mChildren; // List of children.
    SceneNode *mParent; // Parent of this node

    QList<SharedRenderable> mAttachedObjects;

    // Mutable since they're only used for caching. Always call worldMatrix() internally.
    mutable bool mWorldMatrixInvalid;
    mutable Matrix4 mWorldMatrix;
    mutable bool mWorldBoundingBoxInvalid;
    mutable Box3d mWorldBoundingBox;
    mutable bool mBoundingBoxInvalid;
    mutable Box3d mBoundingBox;
    mutable bool mFullTransformInvalid;
    mutable Matrix4 mFullTransform;
};

inline const Box3d &SceneNode::boundingBox() const
{
    if (mBoundingBoxInvalid) {
        updateBoundingBox();
    }

    return mBoundingBox;
}

inline const Box3d &SceneNode::worldBoundingBox() const
{
    if (mWorldBoundingBoxInvalid) {
        mWorldBoundingBox = boundingBox().transformAffine(fullTransform());
        mWorldBoundingBoxInvalid = false;
    }

    return mWorldBoundingBox;    
}

inline const Matrix4 &SceneNode::fullTransform() const
{
    if (mFullTransformInvalid) {
        updateFullTransform();
    }

    return mFullTransform;
}

typedef QSharedPointer<SceneNode> SharedSceneNode;

inline const Vector4 &SceneNode::position() const
{
    return mPosition;
}

inline const Vector4 &SceneNode::scale() const
{
    return mScale;
}

inline const Quaternion &SceneNode::rotation() const
{
    return mRotation;
}

inline bool SceneNode::isInteractive() const
{
    return mInteractive;
}

inline bool SceneNode::isAnimated() const
{
    return mAnimated;
}

inline const Matrix4 &SceneNode::worldMatrix() const
{
    if (mWorldMatrixInvalid) {
        mWorldMatrix = Matrix4::transformation(mScale, mRotation, mPosition);
        mWorldMatrixInvalid = false;        
    }

    return mWorldMatrix;
}

inline Scene *SceneNode::scene() const
{
    return mScene;
}

inline void SceneNode::setPosition(const Vector4 &position)
{
    mPosition = position;
    mWorldMatrixInvalid = true;
    mFullTransformInvalid = true;
    mWorldBoundingBoxInvalid = true;
}

inline void SceneNode::setRotation(const Quaternion &rotation)
{
    mRotation = rotation;
    mWorldMatrixInvalid = true;
    mFullTransformInvalid = true;
    mWorldBoundingBoxInvalid = true;
}

inline void SceneNode::setScale(const Vector4 &scale)
{
    mScale = scale;
    mWorldMatrixInvalid = true;
    mFullTransformInvalid = true;
    mWorldBoundingBoxInvalid = true;
}

inline void SceneNode::setInteractive(bool interactive)
{
    mInteractive = interactive;
}

inline void SceneNode::setAnimated(bool animated)
{
    mAnimated = animated;
}

inline void SceneNode::setScene(Scene *scene) 
{
    mScene = scene;
}

inline const QList<SharedRenderable> &SceneNode::attachedObjects() const
{
    return mAttachedObjects;
}

}

#endif // SCENENODE_H
