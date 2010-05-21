#ifndef LIGHTING_H
#define LIGHTING_H

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {
    
class Light
{
public:
    Light();
		
	bool load(const QDomElement &element);

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

    const Vector4 &position() const {
        return mPosition;
    }

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

private:
	Type mType;
	float mRange;
    float mPhi;
    float mTheta;
    float mAttenuation;
	Vector4 mPosition;
	Vector4 mDirection; // Invalid for point lights
	Vector4 mColor;

    bool parseColor(const QDomElement &element, Vector4 &result) const;
    bool parsePosition(const QDomElement &element, Vector4 &result) const;
};

inline Light::Light() : mColor(0,0,0,0)
{
}

} 

#endif // LIGHTING_H
