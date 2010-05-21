
#include "renderstates.h"
#include "util.h"
#include "lighting.h"

namespace EvilTemple {

bool Light::parseColor(const QDomElement &element, Vector4 &result) const
{
    bool ok;

    int r = element.attribute("red").toInt(&ok);
    if (!ok)
        return false;

    int g = element.attribute("green").toInt(&ok);
    if (!ok)
        return false;

    int b = element.attribute("blue").toInt(&ok);
    if (!ok)
        return false;

    result.setX(r / 255.0f);
    result.setY(g / 255.0f);
    result.setZ(b / 255.0f);
    result.setW(1);

    return true;
}

bool Light::parsePosition(const QDomElement &element, Vector4 &result) const
{
    bool ok;

    result.setX(element.attribute("x").toFloat(&ok));
    if (!ok) {
        return false;
    }

    result.setY(element.attribute("y").toFloat(&ok));
    if (!ok) {
        return false;
    }

    result.setZ(element.attribute("z").toFloat(&ok));
    if (!ok) {
        return false;
    }

    return true;
}

bool Light::load(const QDomElement &element)
{
	if (element.nodeName() == "pointLight") {
		mType = Point;
	} else if (element.nodeName() == "spotLight") {
		mType = Spot;
	} else if (element.nodeName() == "directionalLight") {
		mType = Directional;
	} else {
		qWarning("Light has invalid type: %s.", qPrintable(element.nodeName()));
		return false;
	}

    bool ok;
    mRange = element.attribute("range").toFloat(&ok);
    if (!ok) {
        qWarning("Light has invalid range: %s.", qPrintable(element.attribute("range")));
        return false;
    }

    if (mType == Spot) {
        mPhi = deg2rad(element.attribute("phi").toFloat(&ok));
        if (!ok) {
            qWarning("Light has invalid phi angle: %s", qPrintable(element.attribute("phi")));
            return false;
        }
        mTheta = deg2rad(element.attribute("theta").toFloat(&ok));
        if (!ok) {
            qWarning("Light has invalid theta angle: %s", qPrintable(element.attribute("theta")));
            return false;
        }
    }

    if (mType != Directional) {
        mAttenuation = deg2rad(element.attribute("attenuation").toFloat(&ok));
        if (!ok) {
            qWarning("Light has invalid attenuation value: %s", qPrintable(element.attribute("attenuation")));
            return false;
        }
    }

    QDomElement position = element.firstChildElement("position");
        
    if (position.isNull()) {
        qWarning("Light lacks position element.");
        return false;
    }

    if (!parsePosition(position, mPosition)) {
        qWarning("Light has invalid position element.");
        return false;
    }

    mPosition.setW(1);

    // Directional lights need a direction
    if (mType == Directional) {
        QDomElement direction = element.firstChildElement("direction");
        
        if (direction.isNull()) {
            qWarning("Directional light lacks direction element.");
            return false;
        }

        if (!parsePosition(direction, mDirection)) {
            qWarning("Directional light lacks direction element.");
            return false;
        }

        mDirection.setW(0);
    }

    QDomElement color = element.firstChildElement("color");
    if (!color.isNull()) {
        if (!parseColor(color, mColor)) {
            qWarning("Light has invalid diffuse color element.");
            return false;
        }
    }
    
    return true;
}

}
