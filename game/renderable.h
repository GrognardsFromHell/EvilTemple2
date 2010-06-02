
#ifndef RENDERABLE_H
#define RENDERABLE_H

#include <QObject>
#include <QSharedPointer>

#include "renderstates.h"
#include "renderqueue.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class SceneNode;
class RenderStates;

class Renderable : public QObject {
Q_OBJECT
public:
    Renderable();
    virtual ~Renderable();
    
    virtual void elapseTime(float secondsElapsed);

    virtual void render(RenderStates &renderStates) = 0;

    virtual const Box3d &boundingBox() = 0;

    virtual const Matrix4 &worldTransform() const;

    SceneNode *parentNode() const;
    void setParentNode(SceneNode *parent);

    virtual IntersectionResult intersect(const Ray3d &ray) const;

    bool isAnimated() const;
    void setAnimated(bool animated);

    virtual void mousePressEvent();
    virtual void mouseReleaseEvent();
    virtual void mouseEnterEvent();
    virtual void mouseLeaveEvent();

    RenderQueue::Category renderCategory() const;
    void setRenderCategory(RenderQueue::Category category);

signals:
    void mousePressed();
    void mouseReleased();
    void mouseEnter();
    void mouseLeave();

protected:
    SceneNode *mParentNode;
    bool mAnimated;
    RenderQueue::Category mRenderCategory; // In which category should the content of this node be rendered.

private:
    Q_DISABLE_COPY(Renderable)
};

inline bool Renderable::isAnimated() const
{
    return mAnimated;
}

inline void Renderable::setAnimated(bool animated)
{
    mAnimated = animated;
}

inline void Renderable::setParentNode(SceneNode *parent)
{
    // TODO: Detach from old parent here?
    mParentNode = parent;
}

inline SceneNode *Renderable::parentNode() const
{
    return mParentNode;
}

inline RenderQueue::Category Renderable::renderCategory() const
{
    return mRenderCategory;
}

inline void Renderable::setRenderCategory(RenderQueue::Category category)
{
    Q_ASSERT(category >= RenderQueue::Default && category <= RenderQueue::Count);
    mRenderCategory = category;
}


typedef QSharedPointer<Renderable> SharedRenderable;

};

#endif // RENDERABLE_H
