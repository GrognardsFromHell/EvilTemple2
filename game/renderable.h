
#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "renderstates.h"
#include "modelfile.h"

namespace EvilTemple {

class SceneNode;

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

signals:
    void mousePressed();
    void mouseReleased();
    void mouseEnter();
    void mouseLeave();

protected:
    SceneNode *mParentNode;
    bool mAnimated;

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

typedef QSharedPointer<Renderable> SharedRenderable;

};

#endif // RENDERABLE_H
