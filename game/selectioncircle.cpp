#include "selectioncircle.h"
#include "scenenode.h"
#include "materials.h"
#include "drawhelper.h"

namespace EvilTemple {

struct SelectionDrawStrategy : public DrawStrategy {

    SelectionDrawStrategy(const Vector4 &color, float rotation, int type)
        : mColor(color), mRotation(rotation), mType(type)
    {
    }

    void draw(const RenderStates &renderStates, MaterialPassState &state) const
    {
        int colorLoc = state.program->uniformLocation("color");
        if (colorLoc != -1)
            glUniform4fv(colorLoc, 1, mColor.data());
        int rotationLoc = state.program->uniformLocation("rotation");
        if (rotationLoc != -1)
            glUniform1f(rotationLoc, mRotation);
        int typeLoc = state.program->uniformLocation("type");
        if (typeLoc != -1)
            glUniform1i(typeLoc, mType);

        SAFE_GL(glDrawArrays(GL_QUADS, 0, 4));
    }

    const Vector4 &mColor;
    float mRotation;
    int mType;

};

SelectionCircle::SelectionCircle(Materials *materials)
    : mColor(1, 1, 1, 1),
    mRotation(0),
    mRadius(20),
    mBuffersInvalid(true),
    mSelected(false),
    mHovering(false),
    mHeight(1),
    mMouseDown(false),
    mMaterial(materials->load(":/material/selection_material.xml"))
{
    updateBoundingBox();
}

void SelectionCircle::render(RenderStates &renderStates, MaterialState *overrideMaterial)
{

    if (!mHovering && !mSelected || !mMaterial)
        return;

    if (mBuffersInvalid) {
        Vector4 vertices[4] = {
            Vector4(mRadius, 0, -mRadius, 1),
            Vector4(mRadius, 0, mRadius, 1),
            Vector4(-mRadius, 0, mRadius, 1),
            Vector4(-mRadius, 0, -mRadius, 1),
        };

        mVertices.upload(vertices, sizeof(vertices));

        float texCoords[8] = {
            2, -1,
            2, 1,
            0, 1,
            0, -1,
        };

        mTexCoords.upload(texCoords, sizeof(texCoords));
        mBuffersInvalid = false;
    }

    ModelBufferSource bufferSource(mVertices.bufferId(), 0, mTexCoords.bufferId());

    int type = mMouseDown ? 1 : 0;

    if (mSelected)
        type = 2;

    SelectionDrawStrategy drawer(mColor, mRotation, type);

    DrawHelper<SelectionDrawStrategy, ModelBufferSource> drawHelper;
    drawHelper.draw(renderStates, mMaterial.data(), drawer, bufferSource);
}

const Box3d &SelectionCircle::boundingBox()
{
    return mBoundingBox;
}

void SelectionCircle::mousePressEvent(QMouseEvent *evt)
{
    mMouseDown = true;
    Renderable::mousePressEvent(evt);
}

void SelectionCircle::mouseReleaseEvent(QMouseEvent *evt)
{
    mMouseDown = false;
    Renderable::mouseReleaseEvent(evt);
}

void SelectionCircle::mouseEnterEvent(QMouseEvent *evt)
{
    mHovering = true;
    Renderable::mouseEnterEvent(evt);
}

void SelectionCircle::mouseLeaveEvent(QMouseEvent *evt)
{
    mHovering = false;
    Renderable::mouseLeaveEvent(evt);
}

void SelectionCircle::elapseTime(float secondsElapsed)
{
    mRotation += secondsElapsed;
}

void SelectionCircle::updateBoundingBox()
{
    Vector4 extent(mRadius, 0, mRadius, 0);
    mBoundingBox.setMinimum(-extent);
    mBoundingBox.setMaximum(extent + Vector4(0, mHeight, 0, 0));
}

IntersectionResult SelectionCircle::intersect(const Ray3d &ray) const
{
    IntersectionResult result;
    result.intersects = false;
    result.distance = std::numeric_limits<float>::infinity();

    if (ray.intersects(mBoundingBox)) {
        result.intersects = true;
        result.distance = 1;
    }

    return result;
}

}
