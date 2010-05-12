
#include "renderstates.h"
#include "materialstate.h"

namespace EvilTemple {

RenderStates::RenderStates()
    : mWorldViewProjectionMatrixBinder(new ReferenceBinder<Matrix4>(mWorldViewProjectionMatrix)),
    mWorldViewMatrixBinder(new ReferenceBinder<Matrix4>(mWorldViewMatrix)),
    mWorldMatrixBinder(new ReferenceBinder<Matrix4>(mWorldMatrix)),
    mProjectionMatrixBinder(new ReferenceBinder<Matrix4>(mProjectionMatrix)),
    mViewMatrixBinder(new ReferenceBinder<Matrix4>(mViewMatrix)),
    mViewProjectionMatrixBinder(new ReferenceBinder<Matrix4>(mViewProjectionMatrix)),
    mScreenViewport(0, 0, 0, 0)
{
    mWorldMatrix.setToIdentity();
    mViewMatrix.setToIdentity();
    mProjectionMatrix.setToIdentity();
    mViewProjectionMatrix.setToIdentity();
    mWorldViewMatrix.setToIdentity();
    mWorldViewProjectionMatrix.setToIdentity();
}

RenderStates::~RenderStates()
{
}

void RenderStates::updateScreenViewport()
{
    float transX = - mViewMatrix(0, 3);
    float transY = mViewMatrix(1, 3);

    float w = (1.0f / mProjectionMatrix(0, 0));
    float h = (1.0f / mProjectionMatrix(1, 1));

    mScreenViewport = Box2d(transX - w, transY - h, transX + w, transY + h);
}

const UniformBinder *RenderStates::getStateBinder(const QString &semantic) const
{
    if (semantic == "WorldViewProjection") {
        return mWorldViewProjectionMatrixBinder.data();
   } else if (semantic == "WorldView") {
       return mWorldViewMatrixBinder.data();
   } else if (semantic == "World") {
       return mWorldMatrixBinder.data();
   } else if (semantic == "Projection") {
       return mProjectionMatrixBinder.data();
   } else if (semantic == "View") {
       return mViewMatrixBinder.data();
   } else if (semantic == "ViewProjection") {
       return mViewProjectionMatrixBinder.data();
   } else {
       return NULL;
   }
}

}