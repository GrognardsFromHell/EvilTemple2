
#include "geometryrenderables.h"
#include "materials.h"
#include "drawhelper.h"
#include "gamemath_streams.h"

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
    Vector4 extent(mRadius, mRadius, mRadius, 0);
    mBoundingBox.setMinimum(- extent);
    mBoundingBox.setMaximum(extent);
    mBoundingBoxInvalid = false;
}

void LineRenderable::render(RenderStates &renderStates, MaterialState *overrideMaterial)
{
    glDisable(GL_DEPTH_TEST);
    glColor4f(255, 255, 255, 255);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(renderStates.projectionMatrix().data());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(renderStates.worldViewMatrix().data());

    glBegin(GL_LINES);
    foreach (const Line &line, mLines) {
        glVertex4fv(line.first.data());
        glVertex4fv(line.second.data());
    }

    glEnd();

    glEnable(GL_DEPTH_TEST);
}

void LineRenderable::addLine(const Vector4 &start, const Vector4 &end)
{
    mBoundingBox.merge(start);
    mBoundingBox.merge(end);
    mLines.append(Line(start, end));
}

const Box3d &LineRenderable::boundingBox()
{
    return mBoundingBox;
}

struct DecoratedLineDrawStrategy : public DrawStrategy {

    DecoratedLineDrawStrategy(int count, const Vector4 &color)
        : mCount(count), mColor(color)
    {
    }

    void draw(const RenderStates &renderStates, MaterialPassState &state) const
    {
        int location = state.program->uniformLocation("color");
        if (location != -1)
            bindUniform(location, mColor);

        SAFE_GL(glDrawArrays(GL_QUADS, 0, mCount));
    }

    int mCount;
    Vector4 mColor;

};

DecoratedLineRenderable::DecoratedLineRenderable(Materials *materials)
    : mBuffersInvalid(true),
    mColor(1, 0, 0, 1),
    mMaterial(materials->load("materials/decorated_line.xml")),
    mLineWidth(16)
{
}

void DecoratedLineRenderable::render(RenderStates &renderStates, MaterialState *overrideMaterial)
{
    if (!mMaterial)
        return;

    if (mBuffersInvalid)
        updateBuffers();

    ModelBufferSource bufferSource(mVertices.bufferId(), 0, mTexCoords.bufferId());

    DecoratedLineDrawStrategy drawer(mLines.count() * 4, mColor);

    DrawHelper<DecoratedLineDrawStrategy, ModelBufferSource> drawHelper;
    drawHelper.draw(renderStates, mMaterial.data(), drawer, bufferSource);
}

void DecoratedLineRenderable::addLine(const Vector4 &start, const Vector4 &end)
{
    if (mBoundingBox.isNull()) {
        mBoundingBox.setMinimum(start);
        mBoundingBox.setMaximum(start);
        mBoundingBox.merge(end);
    } else {
        mBoundingBox.merge(start);
        mBoundingBox.merge(end);
    }
    mLines.append(Line(start, end));
}

void DecoratedLineRenderable::clearLines()
{
    mBoundingBox.setMinimum(Vector4(0,0,0,0));
    mBoundingBox.setMaximum(Vector4(0,0,0,0));
    mLines.clear();
}

void DecoratedLineRenderable::updateBuffers()
{
    QByteArray vertexData(sizeof(Vector4) * 4 * mLines.count(), Qt::Uninitialized);
    QDataStream vertexStream(&vertexData, QIODevice::WriteOnly);
    vertexStream.setByteOrder(QDataStream::LittleEndian);
    vertexStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    Vector4 planeNormal(0, 1, 0, 0);

    foreach (const Line &line, mLines) {
        Vector4 dir = line.second - line.first;

        Vector4 perpenNormal = dir.cross(planeNormal).normalized() * mLineWidth * 0.5f;

        vertexStream << (line.first + perpenNormal)
                     << (line.first - perpenNormal)
                     << (line.second - perpenNormal)
                     << (line.second + perpenNormal);
    }

    mVertices.upload(vertexData.constData(), vertexData.size());

    float texCoords[8] = {
        1, 0,
        1, 1,
        0, 1,
        0, 0,
    };

    QByteArray texCoordsData(8 * sizeof(float) * mLines.count(), Qt::Uninitialized);

    for (int i = 0; i < mLines.count(); ++i) {
                memcpy(texCoordsData.data() + i * 8 * sizeof(float), texCoords, 8 * sizeof(float));
    }

    mTexCoords.upload(texCoordsData.constData(), texCoordsData.size());

    mBuffersInvalid = false;
}

const Box3d &DecoratedLineRenderable::boundingBox()
{
    return mBoundingBox;
}

}


