
#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include "matrix4.h"

using namespace GameMath;

namespace EvilTemple {
/**
 * Encapsulates various render state settings that influence the rendering of the scene.
 */
class RenderStates {
public:
	RenderStates();

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
private:
	Matrix4 mWorldMatrix;
	Matrix4 mViewMatrix;
	Matrix4 mProjectionMatrix;

	Matrix4 mWorldViewMatrix;
	Matrix4 mViewProjectionMatrix;
	Matrix4 mWorldViewProjectionMatrix;
};

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
}

inline void RenderStates::setProjectionMatrix(const Matrix4 &projectionMatrix)
{
	mProjectionMatrix = projectionMatrix;
	mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
	mWorldViewProjectionMatrix = projectionMatrix * mWorldViewMatrix;
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
