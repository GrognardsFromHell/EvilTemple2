#ifndef SELECTIONCIRCLE_H
#define SELECTIONCIRCLE_H

#include "renderable.h"
#include "materialstate.h"
#include "vertexbufferobject.h"

namespace EvilTemple {

class Materials;

class SelectionCircle : public Renderable
{
Q_OBJECT
Q_PROPERTY(float radius READ radius WRITE setRadius)
Q_PROPERTY(float height READ height WRITE setHeight)
Q_PROPERTY(Vector4 color READ color WRITE setColor)
Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
Q_PROPERTY(bool hovering READ isHovering WRITE setHovering)
public:
    SelectionCircle(Materials *materials);

    void render(RenderStates &renderStates, MaterialState *overrideMaterial = NULL);

    const Box3d &boundingBox();

    void mousePressEvent(QMouseEvent *evt);
    void mouseReleaseEvent(QMouseEvent *evt);
    void mouseEnterEvent(QMouseEvent *evt);
    void mouseLeaveEvent(QMouseEvent *evt);

    const Vector4 &color() const;
    void setColor(const Vector4 &color);

    float radius() const;
    void setRadius(float radius);

    float height() const;
    void setHeight(float height);

    bool isSelected() const;
    void setSelected(bool selected);

    bool isHovering() const;
    void setHovering(bool hovering);

    IntersectionResult intersect(const Ray3d &ray) const;

public slots:
    void elapseTime(float secondsElapsed);

private:
    void updateBoundingBox();

    VertexBufferObject mVertices;
    VertexBufferObject mTexCoords;

    bool mBuffersInvalid;
    bool mSelected;
    bool mHovering;
    bool mMouseDown;
    Vector4 mColor;
    float mRadius;
    float mHeight;
    float mRotation;
    Box3d mBoundingBox;
    SharedMaterialState mMaterial;
};

inline const Vector4 &SelectionCircle::color() const
{
    return mColor;
}

inline void SelectionCircle::setColor(const Vector4 &color)
{
    mColor = color;
}

inline float SelectionCircle::radius() const
{
    return mRadius;
}

inline void SelectionCircle::setRadius(float radius)
{
    mRadius = radius;
    mBuffersInvalid = true;
    updateBoundingBox();
}

inline bool SelectionCircle::isSelected() const
{
    return mSelected;
}

inline void SelectionCircle::setSelected(bool selected)
{
    mSelected = selected;
}

inline void SelectionCircle::setHeight(float height)
{
    mHeight = height;
}

inline float SelectionCircle::height() const
{
    return mHeight;
}

inline bool SelectionCircle::isHovering() const
{
    return mHovering;
}

inline void SelectionCircle::setHovering(bool hovering)
{
    mHovering = hovering;
}

Q_DECLARE_METATYPE(SelectionCircle*)

}

#endif // SELECTIONCIRCLE_H
