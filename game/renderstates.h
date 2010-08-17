
#ifndef RENDERSTATES_H
#define RENDERSTATES_H

#include "gameglobal.h"

#include <QtCore/QString>
#include <QtCore/QScopedPointer>
#include <QtCore/QList>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class UniformBinder;
class Light;

/**
 * Encapsulates various render state settings that influence the rendering of the scene.
 */
class GAME_EXPORT RenderStates {
public:
    RenderStates();
    ~RenderStates();

    const Matrix4 &worldMatrix() const;
    const Matrix4 &viewMatrix() const;
    const Matrix4 &viewInverseMatrix() const;
    const Matrix4 &projectionMatrix() const;

    void setWorldMatrix(const Matrix4 &worldMatrix);
    void setViewMatrix(const Matrix4 &viewMatrix);
    void setProjectionMatrix(const Matrix4 &projectionMatrix);
    void setTextureAnimationTime(float t);

    // Aggregate matrices, these are calculated from the three base matrices
    const Matrix4 &worldViewMatrix() const;
    const Matrix4 &worldViewProjectionMatrix() const;
    const Matrix4 &viewProjectionMatrix() const;
    float textureAnimationTime() const;

    /**
    Returns the active lights in the scene.
    */
    const QList<const Light*> &activeLights() const;
    void setActiveLights(const QList<const Light*> &activeLights);

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

    Matrix4 mWorldInverseMatrix;
    QScopedPointer<UniformBinder> mWorldInverseMatrixBinder;

    Matrix4 mViewMatrix;
    QScopedPointer<UniformBinder> mViewMatrixBinder;

    Matrix4 mViewInverseMatrix;
    QScopedPointer<UniformBinder> mViewInverseMatrixBinder;

    Matrix4 mProjectionMatrix;
    QScopedPointer<UniformBinder> mProjectionMatrixBinder;

    Matrix4 mWorldViewMatrix;
    QScopedPointer<UniformBinder> mWorldViewMatrixBinder;

    Matrix4 mWorldViewInverseMatrix;
    QScopedPointer<UniformBinder> mWorldViewInverseMatrixBinder;

    Matrix4 mWorldViewInverseTransposeMatrix;
    QScopedPointer<UniformBinder> mWorldViewInverseTransposeMatrixBinder;

    Matrix4 mViewProjectionMatrix;
    QScopedPointer<UniformBinder> mViewProjectionMatrixBinder;

    Matrix4 mWorldViewProjectionMatrix;
    QScopedPointer<UniformBinder> mWorldViewProjectionMatrixBinder;

    float mTextureAnimationTime;
    QScopedPointer<UniformBinder> mTextureAnimationTimeBinder;

    QList<const Light*> mActiveLights;
};

inline void RenderStates::setTextureAnimationTime(float t)
{
    mTextureAnimationTime = t;
}

inline float RenderStates::textureAnimationTime() const
{
    return mTextureAnimationTime;
}

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
    mWorldInverseMatrix = worldMatrix.inverted(); // Should probably be deferred
    mWorldViewInverseMatrix = mWorldInverseMatrix * mViewInverseMatrix;  // Should probably be deferred
    mWorldViewInverseTransposeMatrix = mWorldViewInverseMatrix.transposed();
    mWorldViewMatrix = mViewMatrix * worldMatrix;
    mWorldViewProjectionMatrix = mProjectionMatrix * mWorldViewMatrix;
}

inline void RenderStates::setViewMatrix(const Matrix4 &viewMatrix)
{
    mViewMatrix = viewMatrix;
    mViewInverseMatrix = viewMatrix.inverted();  // Should probably be deferred
    mWorldViewMatrix = viewMatrix * mWorldMatrix;
    mWorldViewInverseMatrix = mWorldViewMatrix.inverted();  // Should probably be deferred
    mWorldViewInverseTransposeMatrix = mWorldViewInverseMatrix.transposed();
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

inline const Matrix4 &RenderStates::viewInverseMatrix() const
{
    return mViewInverseMatrix;
}

inline const Matrix4 &RenderStates::worldViewProjectionMatrix() const
{
    return mWorldViewProjectionMatrix;
}

inline const QList<const Light*> &RenderStates::activeLights() const
{
    return mActiveLights;
}

inline void RenderStates::setActiveLights(const QList<const Light*> &activeLights)
{
    mActiveLights = activeLights;
}

}

#endif
