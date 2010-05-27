
#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "renderstates.h"
#include "modelfile.h"

namespace EvilTemple {

class SceneNode;

class Renderable {
public:
    Renderable();
    virtual ~Renderable();
    
    virtual void elapseTime(float secondsElapsed);

    virtual void render(RenderStates &renderStates) = 0;

    virtual const Box3d &boundingBox() = 0;

    virtual const Matrix4 &worldTransform() const;

    void setParentNode(const SceneNode *parent);

    bool isAnimated() const;
    void setAnimated(bool animated);

protected:
    const SceneNode *mParentNode;
    bool mAnimated;

};

inline bool Renderable::isAnimated() const
{
    return mAnimated;
}

inline void Renderable::setAnimated(bool animated)
{
    mAnimated = animated;
}

inline void Renderable::setParentNode(const SceneNode *parent)
{
    // TODO: Detach from old parent here?
    mParentNode = parent;
}

typedef QSharedPointer<Renderable> SharedRenderable;

};

#endif // RENDERABLE_H
