#ifndef GEOMETRYRENDERABLES_H
#define GEOMETRYRENDERABLES_H

#include "renderable.h"
#include "materialstate.h"
#include "vertexbufferobject.h"

namespace EvilTemple {

class Materials;

class MovementIndicator : public Renderable {
    Q_OBJECT
    Q_PROPERTY(bool drawLine READ isDrawLine WRITE setDrawLine)
    Q_PROPERTY(Vector4 source READ source WRITE setSource)
    Q_PROPERTY(float radius READ radius WRITE setRadius)
    Q_PROPERTY(float circleWidth READ circleWidth WRITE setCircleWidth)
    Q_PROPERTY(float lineWidth READ lineWidth WRITE setLineWidth)
    Q_PROPERTY(float secondaryLineLength READ secondaryLineLength WRITE setSecondaryLineLength)
    Q_PROPERTY(Vector4 lineColor READ lineColor WRITE setLineColor)
    Q_PROPERTY(Vector4 secondaryLineColor READ secondaryLineColor WRITE setSecondaryLineColor)
    Q_PROPERTY(Vector4 circleColor READ circleColor WRITE setCircleColor)
    Q_PROPERTY(Vector4 fillColor READ fillColor WRITE setFillColor)
    Q_PROPERTY(bool pulsating READ isPulsating WRITE setPulsating)
public:

    MovementIndicator(Materials *materials);

    bool isDrawLine() const;
    void setDrawLine(bool drawLine);

    const Vector4 &source() const;
    void setSource(const Vector4 &source);

    float radius() const;
    void setRadius(float radius);

    float circleWidth() const;
    void setCircleWidth(float width);

    float lineWidth() const;
    void setLineWidth(float width);

    float secondaryLineLength() const;
    void setSecondaryLineLength(float factor);

    const Vector4 &lineColor() const;
    void setLineColor(const Vector4 &color);

    const Vector4 &secondaryLineColor() const;
    void setSecondaryLineColor(const Vector4 &color);

    const Vector4 &circleColor() const;
    void setCircleColor(const Vector4 &color);

    const Vector4 &fillColor() const;
    void setFillColor(const Vector4 &color);

    bool isPulsating() const;
    void setPulsating(bool pulsating);

    float pulseScaling() const;

    void render(RenderStates &renderStates, MaterialState *overrideMaterial);

    const Box3d &boundingBox();

    void elapseTime(float secondsElapsed);

private:
    bool mDrawLine;
    Vector4 mSource;
    float mRadius;
    float mCircleWidth;
    float mLineWidth;
    float mSecondaryLineLength;
    Vector4 mLineColor;
    Vector4 mSecondaryLineColor;
    Vector4 mCircleColor;
    Vector4 mFillColor;
    bool mPulsating;
    float mPulseScaling;

    bool mBoundingBoxInvalid;
    Box3d mBoundingBox;

    bool mBuffersInvalid;
    SharedMaterialState mCircleMaterial;
    VertexBufferObject mVertices;
    VertexBufferObject mTexCoords;

    void updateBuffers();
    void updateBoundingBox();
};

inline float MovementIndicator::pulseScaling() const
{
    return mPulseScaling;
}

inline bool MovementIndicator::isDrawLine() const
{
    return mDrawLine;
}

inline void MovementIndicator::setDrawLine(bool drawLine)
{
    mDrawLine = drawLine;
    mBoundingBoxInvalid = true;
}

inline const Vector4 &MovementIndicator::source() const
{
    return mSource;
}

inline void MovementIndicator::setSource(const Vector4 &source)
{
    mSource = source;
    mBoundingBoxInvalid = true;
}

inline float MovementIndicator::radius() const
{
    return mRadius;
}

inline void MovementIndicator::setRadius(float radius)
{
    mRadius = radius;
    mBoundingBoxInvalid = true;
    mBuffersInvalid = true;
}

inline float MovementIndicator::circleWidth() const
{
    return mCircleWidth;
}

inline void MovementIndicator::setCircleWidth(float width)
{
    mCircleWidth = width;
    mBoundingBoxInvalid = true;
    mBuffersInvalid = true;
}

inline float MovementIndicator::lineWidth() const
{
    return mLineWidth;
}

inline void MovementIndicator::setLineWidth(float width)
{
    mLineWidth = width;
    mBoundingBoxInvalid = true;
    mBuffersInvalid = true;
}

inline float MovementIndicator::secondaryLineLength() const
{
    return mSecondaryLineLength;
}

inline void MovementIndicator::setSecondaryLineLength(float factor)
{
    mSecondaryLineLength = factor;
}

inline const Vector4 &MovementIndicator::lineColor() const
{
    return mLineColor;
}

inline void MovementIndicator::setLineColor(const Vector4 &color)
{
    mLineColor = color;
}

inline const Vector4 &MovementIndicator::secondaryLineColor() const
{
    return mSecondaryLineColor;
}

inline void MovementIndicator::setSecondaryLineColor(const Vector4 &color)
{
    mSecondaryLineColor = color;
}


inline const Vector4 &MovementIndicator::circleColor() const
{
    return mCircleColor;
}

inline void MovementIndicator::setCircleColor(const Vector4 &color)
{
    mCircleColor = color;
}


inline const Vector4 &MovementIndicator::fillColor() const
{
    return mFillColor;
}

inline void MovementIndicator::setFillColor(const Vector4 &color)
{
    mFillColor = color;
}


inline bool MovementIndicator::isPulsating() const
{
    return mPulsating;
}

inline void MovementIndicator::setPulsating(bool pulsating)
{
    mPulsating = pulsating;
    mBoundingBoxInvalid = true;
}

inline const Box3d &MovementIndicator::boundingBox()
{
    if (mBoundingBoxInvalid)
        updateBoundingBox();
    return mBoundingBox;
}

/**
  A renderable that will draw a list of lines (in model space).
  The main use of this renderable is to show debugging information.
  */
class LineRenderable : public Renderable
{
Q_OBJECT
public:
    void render(RenderStates &renderStates, MaterialState *overrideMaterial = NULL);

    const Box3d &boundingBox();

public slots:
    void addLine(const Vector4 &start, const Vector4 &end);

private:
    typedef QPair<Vector4,Vector4> Line;

    QVector<Line> mLines;
    Box3d mBoundingBox;
};

/**
  A renderable that will draw a list of lines (in model space).
  The main use of this renderable is to show debugging information.
  */
class DecoratedLineRenderable : public Renderable
{
Q_OBJECT
Q_PROPERTY(Vector4 color READ color WRITE setColor)
Q_PROPERTY(float lineWidth READ lineWidth WRITE setLineWidth)
public:
    DecoratedLineRenderable(Materials *materials);

    void render(RenderStates &renderStates, MaterialState *overrideMaterial = NULL);

    const Box3d &boundingBox();

    const Vector4 &color() const;
    void setColor(const Vector4 &color);

    float lineWidth() const;
    void setLineWidth(float width);

public slots:
    void addLine(const Vector4 &start, const Vector4 &end);

private:
    typedef QPair<Vector4,Vector4> Line;

    bool mBuffersInvalid;
    VertexBufferObject mVertices;
    VertexBufferObject mTexCoords;
    SharedMaterialState mMaterial;

    float mLineWidth;
    Vector4 mColor;
    QVector<Line> mLines;
    Box3d mBoundingBox;

    void updateBuffers();
};

inline float DecoratedLineRenderable::lineWidth() const
{
    return mLineWidth;
}

inline void DecoratedLineRenderable::setLineWidth(float width)
{
    mLineWidth = width;
    mBuffersInvalid = true;
}

inline const Vector4 &DecoratedLineRenderable::color() const
{
    return mColor;
}

inline void DecoratedLineRenderable::setColor(const Vector4 &color)
{
    mColor = color;
}

}

Q_DECLARE_METATYPE(EvilTemple::LineRenderable*)
Q_DECLARE_METATYPE(EvilTemple::MovementIndicator*)

#endif // GEOMETRYRENDERABLES_H
