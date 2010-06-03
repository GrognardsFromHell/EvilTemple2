
#include "renderable.h"
#include "scenenode.h"

namespace EvilTemple {

Renderable::Renderable()
    : mParentNode(NULL), mRenderCategory(RenderQueue::Default), mDebugging(false)
{
}

Renderable::~Renderable()
{
}

void Renderable::elapseTime(float secondsElapsed)
{
}

const Matrix4 &Renderable::worldTransform() const
{
    Q_ASSERT(mParentNode);
    return mParentNode->fullTransform();
}

IntersectionResult Renderable::intersect(const Ray3d &ray) const
{
    Q_UNUSED(ray);
    IntersectionResult result;
    result.intersects = false;
    return result;
}

void Renderable::mousePressEvent()
{
    emit mousePressed();
}

void Renderable::mouseReleaseEvent()
{
    emit mouseReleased();
}

void Renderable::mouseEnterEvent()
{
    emit mouseEnter();
}

void Renderable::mouseLeaveEvent()
{
    emit mouseLeave();
}

};
