
#include "renderstate.h"

RenderStates::RenderStates()
{
	mWorldMatrix.setToIdentity();
	mViewMatrix.setToIdentity();
	mProjectionMatrix.setToIdentity();
	mViewProjectionMatrix.setToIdentity();
	mWorldViewMatrix.setToIdentity();
	mWorldViewProjectionMatrix.setToIdentity();
}
