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
Q_PROPERTY(Vector4 color READ color WRITE setColor)
Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
public:
    SelectionCircle(Materials *materials);

    void render(RenderStates &renderStates);

    const Box3d &boundingBox();

    void mousePressEvent();
    void mouseReleaseEvent();
    void mouseEnterEvent();
    void mouseLeaveEvent();

    const Vector4 &color() const;
    void setColor(const Vector4 &color);

    float radius() const;
    void setRadius(float radius);

    bool isSelected() const;
    void setSelected(bool selected);

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
    Vector4 mColor;
    float mRadius;
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

Q_DECLARE_METATYPE(SelectionCircle*)

}

#endif // SELECTIONCIRCLE_H
