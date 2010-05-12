
#ifndef RENDERSTATES_H
#define RENDERSTATES_H

#include <QtCore/QString>
#include <QtCore/QScopedPointer>

#include "gamemath.h"

using namespace GameMath;

namespace EvilTemple {

class UniformBinder;

/**
 * Encapsulates various render state settings that influence the rendering of the scene.
 */
class RenderStates {
public:
    RenderStates();
    ~RenderStates();

    const Matrix4 &worldMatrix() const;
    const Matrix4 &viewMatrix() const;
    const Matrix4 &projectionMatrix() const;

    void setWorldMatrix(const Matrix4 &worldMatrix);
    void setViewMatrix(const Matrix4 &viewMatrix);
    void setProjectionMatrix(const Matrix4 &projectionMatrix);

    // Aggregate matrices, these are calculated from the three base matrices
    const Matrix4 &worldViewMatrix() const;
    const Matrix4 &worldViewProjectionMatrix() const;
    const Matrix4 &viewProjectionMatrix() const;

    /**
      Returns a 2D box that encapsulates the viewport in absolute screen coordinates.
      This is useful for culling 2D background images.

      It is updated whenever the projection or view matrices change.
      */
    const Box2d &screenViewport() const;

    /**
      Returns a uniform binder capable of binding a semantic, that is supported
      by this render state object. If the semantic is not supported, NULL is returned.
      The returned object is owned by this render state object and its lifetime will
      depend on this object.
      */
    const UniformBinder *getStateBinder(const QString &semantic) const;
private:
    void updateScreenViewport();
    Box2d mScreenViewport;

    Matrix4 mWorldMatrix;
    QScopedPointer<UniformBinder> mWorldMatrixBinder;

    Matrix4 mViewMatrix;
    QScopedPointer<UniformBinder> mViewMatrixBinder;

    Matrix4 mProjectionMatrix;
    QScopedPointer<UniformBinder> mProjectionMatrixBinder;

    Matrix4 mWorldViewMatrix;
    QScopedPointer<UniformBinder> mWorldViewMatrixBinder;

    Matrix4 mViewProjectionMatrix;
    QScopedPointer<UniformBinder> mViewProjectionMatrixBinder;

    Matrix4 mWorldViewProjectionMatrix;
    QScopedPointer<UniformBinder> mWorldViewProjectionMatrixBinder;
};

inline const Box2d &RenderStates::screenViewport() const
{
    return mScreenViewport;
}

inline const Matrix4 &RenderStates::viewProjectionMatrix() const
{
    return mViewProjectionMatrix;
}

inline const Matrix4 &RenderStates::worldMatrix() const
{
    return mWorldMatrix;
}

inline const Matrix4 &RenderStates::viewMatrix() const
{
    return mViewMatrix;
}

inline const Matrix4 &RenderStates::projectionMatrix() const
{
    return mProjectionMatrix;
}

inline void RenderStates::setWorldMatrix(const Matrix4 &worldMatrix)
{
    mWorldMatrix = worldMatrix;
    mWorldViewMatrix = mViewMatrix * worldMatrix;
    mWorldViewProjectionMatrix = mProjectionMatrix * mWorldViewMatrix;
}

inline void RenderStates::setViewMatrix(const Matrix4 &viewMatrix)
{
    mViewMatrix = viewMatrix;
    mWorldViewMatrix = viewMatrix * mWorldMatrix;
    mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
    mWorldViewProjectionMatrix = mProjectionMatrix * mWorldViewMatrix;
    updateScreenViewport();
}

inline void RenderStates::setProjectionMatrix(const Matrix4 &projectionMatrix)
{
    mProjectionMatrix = projectionMatrix;
    mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
    mWorldViewProjectionMatrix = projectionMatrix * mWorldViewMatrix;
    updateScreenViewport();
}

inline const Matrix4 &RenderStates::worldViewMatrix() const
{
    return mWorldViewMatrix;
}

inline const Matrix4 &RenderStates::worldViewProjectionMatrix() const
{
    return mWorldViewProjectionMatrix;
}

}

#endif
