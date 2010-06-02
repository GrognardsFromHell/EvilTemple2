#ifndef LIGHTING_H
#define LIGHTING_H

#include <QSharedPointer>

#include "renderable.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class LightDebugRenderer;
    
class Light : public Renderable
{
Q_OBJECT
public:
    Light();

    enum Type
    {
        Directional = 1,
        Point,
        Spot,
    };

    Type type() const {
        return mType;
    }

    float range() const {
        return mRange;
    }

    Vector4 position() const;

    const Vector4 &direction() const {
        return mDirection;
    }

    const Vector4 &color() const {
        return mColor;
    }

    float attenuation() const {
        return mAttenuation;
    }

    float phi() const {
        return mPhi;
    }

    float theta() const {
        return mTheta;
    }

    void setType(Type type) {
        mType = type;
    }

    void setRange(float range)
    {
        mRange = range;
        mBoundingBox.setMinimum(Vector4(-range, -range, -range, 1));
        mBoundingBox.setMaximum(Vector4(range, range, range, 1));

        // TODO: Invalidate PARENT'S bounding box
    }

    void setColor(const Vector4 &color)
    {
        mColor = color;
    }

    void setAttenuation(float attenuation)
    {
        mAttenuation = attenuation;
    }

    void setPhi(float phi)
    {
        mPhi = phi;
    }

    void setTheta(float theta)
    {
        mTheta = theta;
    }

    void setDirection(const Vector4 &direction)
    {
        mDirection = direction;
    }

    void render(RenderStates &renderStates);

    const Box3d &boundingBox()
    {
        return mBoundingBox;
    }

    static void setDebugRenderer(LightDebugRenderer *debugRenderer);

private:
    Type mType;
    float mRange;
    float mPhi;
    float mTheta;
    float mAttenuation;
    Vector4 mDirection; // Invalid for point lights
    Vector4 mColor;
    Box3d mBoundingBox;

    static LightDebugRenderer *mDebugRenderer;

    Q_DISABLE_COPY(Light)
};

typedef QSharedPointer<Light> SharedLight;

inline Light::Light() : mColor(0,0,0,0), mDirection(0, 0, 0, 0)
{
    setRenderCategory(RenderQueue::Lights);
}

} 

#endif // LIGHTING_H
