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
    mMaterial(materials->load(":/material/selection_material.xml"))
{
    updateBoundingBox();
}

void SelectionCircle::render(RenderStates &renderStates)
{

    if (!mHovering && !mSelected)
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
    SelectionDrawStrategy drawer(mColor, mRotation, mSelected ? 1 : 0);

    DrawHelper<SelectionDrawStrategy, ModelBufferSource> drawHelper;
    drawHelper.draw(renderStates, mMaterial.data(), drawer, bufferSource);
}

const Box3d &SelectionCircle::boundingBox()
{
    return mBoundingBox;
}

void SelectionCircle::mousePressEvent()
{

}

void SelectionCircle::mouseReleaseEvent()
{

}

void SelectionCircle::mouseEnterEvent()
{
    qDebug("Mouse entered");
    mHovering = true;
}

void SelectionCircle::mouseLeaveEvent()
{
    mHovering = false;
    qDebug("Mouse left");
}

void SelectionCircle::elapseTime(float secondsElapsed)
{
    mRotation += secondsElapsed;
}

void SelectionCircle::updateBoundingBox()
{
    Vector4 extent(mRadius, 1, mRadius, 0);
    mBoundingBox.setMinimum(-extent);
    mBoundingBox.setMaximum(extent);
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
