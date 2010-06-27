
#ifndef RENDERABLE_H
#define RENDERABLE_H

#include <QObject>
#include <QSharedPointer>
#include <QPair>
#include <QVector>

#include "renderstates.h"
#include "renderqueue.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class SceneNode;
class RenderStates;

class Renderable : public QObject, public AlignedAllocation {
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

    void setDebugging(bool debugging);
    bool isDebugging() const;

signals:
    void mousePressed();
    void mouseReleased();
    void mouseEnter();
    void mouseLeave();

protected:
    SceneNode *mParentNode;
    bool mAnimated;
    bool mDebugging;
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

inline void Renderable::setDebugging(bool debugging)
{
    mDebugging = debugging;
}

inline bool Renderable::isDebugging() const
{
    return mDebugging;
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

/**
  A renderable that will draw a list of lines (in model space).
  The main use of this renderable is to show debugging information.
  */
class LineRenderable : public Renderable
{
Q_OBJECT
public:
    void render(RenderStates &renderStates);

    const Box3d &boundingBox();

    void addLine(const Vector4 &start, const Vector4 &end);

private:
    typedef QPair<Vector4,Vector4> Line;

    QVector<Line> mLines;
    Box3d mBoundingBox;
};

uint getActiveRenderables();

};

#endif // RENDERABLE_H
