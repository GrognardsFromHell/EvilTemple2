
#include "geometryrenderables.h"
#include "materials.h"
#include "drawhelper.h"

namespace EvilTemple {


struct MovementIndicatorDrawStrategy : public DrawStrategy {

    MovementIndicatorDrawStrategy(const MovementIndicator *data)
        : mData(data)
    {
    }

    void draw(const RenderStates &renderStates, MaterialPassState &state) const
    {
        int location;

        float extent = mData->radius() + mData->circleWidth();

        location = state.program->uniformLocation("circleColor");
        if (location != -1) {
            bindUniform(location, mData->circleColor());
        }

        location = state.program->uniformLocation("fillColor");
        if (location != -1) {
            if (mData->isPulsating()) {
                Vector4 color = mData->fillColor();
                color.setW(color.w() * (0.5f + sin(mData->pulseScaling() * Pi) * 0.5f));
                bindUniform(location, color);
            } else {
                bindUniform(location, mData->fillColor());
            }
        }

        location = state.program->uniformLocation("radius");
        if (location != -1)
            glUniform1f(location, mData->radius() / extent);

        location = state.program->uniformLocation("circleWidth");
        if (location != -1)
            glUniform1f(location, mData->circleWidth() / extent);

        SAFE_GL(glDrawArrays(GL_QUADS, 0, 4));
    }

    const MovementIndicator *mData;
};

MovementIndicator::MovementIndicator(Materials *materials)
    :
    mDrawLine(false),
    mSource(0, 0, 0, 1),
    mRadius(10),
    mCircleWidth(2),
    mLineWidth(10),
    mSecondaryLineLength(0.5),
    mLineColor(1, 1, 1, 1),
    mSecondaryLineColor(1, 0, 0, 1),
    mCircleColor(0, 0, 1, 1),
    mFillColor(0, 0, 1, 0.5f),
    mPulsating(true),
    mPulseScaling(0),
    mBoundingBoxInvalid(true),
    mBuffersInvalid(true),
    mCircleMaterial(materials->load(":/material/movement_indicator_circle.xml"))
{
    setRenderCategory(Default);
}

void MovementIndicator::render(RenderStates &renderStates, MaterialState *overrideMaterial)
{
    Q_UNUSED(overrideMaterial)

    if (!mCircleMaterial)
        return;

    if (mBuffersInvalid)
        updateBuffers();

    ModelBufferSource bufferSource(mVertices.bufferId(), 0, mTexCoords.bufferId());

    MovementIndicatorDrawStrategy drawer(this);

    DrawHelper<MovementIndicatorDrawStrategy, ModelBufferSource> drawHelper;
    drawHelper.draw(renderStates, mCircleMaterial.data(), drawer, bufferSource);
}

void MovementIndicator::elapseTime(float secondsElapsed)
{
    mPulseScaling += secondsElapsed;
    mPulseScaling -= floor(mPulseScaling);
}

void MovementIndicator::updateBuffers()
{
    qDebug("Updating buffers.");

    float extent = mRadius + mCircleWidth;

    Vector4 vertices[4] = {
        Vector4(extent, 0, -extent, 1),
        Vector4(extent, 0, extent, 1),
        Vector4(-extent, 0, extent, 1),
        Vector4(-extent, 0, -extent, 1),
    };

    mVertices.upload(vertices, sizeof(vertices));

    float texCoords[8] = {
        1, 0,
        1, 1,
        0, 1,
        0, 0,
    };

    mTexCoords.upload(texCoords, sizeof(texCoords));

    mBuffersInvalid = false;
}

void MovementIndicator::updateBoundingBox()
{
    qDebug("Updating bounding box.");

    Vector4 extent(mRadius, mRadius, mRadius, 0);
    mBoundingBox.setMinimum(- extent);
    mBoundingBox.setMaximum(extent);
    mBoundingBoxInvalid = false;
}

}


