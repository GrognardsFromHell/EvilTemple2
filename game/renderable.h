
#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "gameglobal.h"

#include <QObject>
#include <QMetaType>
#include <QPair>
#include <QVector>
#include <QMouseEvent>

#include "renderstates.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class SceneNode;
class RenderStates;
class MaterialState;

class GAME_EXPORT Renderable : public QObject, public AlignedAllocation {
Q_OBJECT
Q_PROPERTY(Box3d boundingBox READ boundingBox)
// Q_PROPERTY(Matrix4 worldTransform READ worldTransform)
Q_PROPERTY(SceneNode* parentNode READ parentNode)
Q_PROPERTY(bool animated READ isAnimated WRITE setAnimated)
Q_PROPERTY(Category renderCategory READ renderCategory WRITE setRenderCategory)
Q_PROPERTY(bool debugging READ isDebugging WRITE setDebugging)
Q_ENUMS(Category)
public:

    enum Category {
        Default = 0,
        ClippingGeometry,
        Lights,
        DebugOverlay,
        StaticGeometry,
        Background,
        FogOfWar,
        Count
    };

    Renderable();
    virtual ~Renderable();

    virtual void render(RenderStates &renderStates, MaterialState *overrideMaterial = NULL) = 0;

    virtual const Box3d &boundingBox() = 0;

    virtual const Matrix4 &worldTransform() const;

    SceneNode *parentNode() const;
    void setParentNode(SceneNode *parent);

    virtual IntersectionResult intersect(const Ray3d &ray) const;

    bool isAnimated() const;
    void setAnimated(bool animated);

    virtual void mousePressEvent(QMouseEvent *mouseEvent);
    virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);
    virtual void mouseEnterEvent(QMouseEvent *mouseEvent);
    virtual void mouseMoveEvent(QMouseEvent *mouseEvent);
    virtual void mouseLeaveEvent(QMouseEvent *mouseEvent);
    virtual void mouseDoubleClickEvent(QMouseEvent *mouseEvent);

    Category renderCategory() const;
    void setRenderCategory(Category category);

    void setDebugging(bool debugging);
    bool isDebugging() const;

public slots:
    virtual void elapseTime(float secondsElapsed);

signals:
    void mousePressed(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
    void mouseDoubleClicked(QMouseEvent *event);
    void mouseEnter(QMouseEvent *event);
    void mouseLeave(QMouseEvent *event);
    void mouseMove(QMouseEvent *event);

protected:
    SceneNode *mParentNode;
    bool mAnimated;
    bool mDebugging;
    Category mRenderCategory; // In which category should the content of this node be rendered.

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

inline Renderable::Category Renderable::renderCategory() const
{
    return mRenderCategory;
}

inline void Renderable::setRenderCategory(Category category)
{
    Q_ASSERT(category >= Default && category <= Count);
    mRenderCategory = category;
}

uint getActiveRenderables();

};

Q_DECLARE_METATYPE(EvilTemple::Renderable*)

#endif // RENDERABLE_H
