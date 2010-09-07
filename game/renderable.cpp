
#include "renderable.h"
#include "scenenode.h"

namespace EvilTemple {

static uint activeRenderables = 0;

uint getActiveRenderables()
{
    return activeRenderables;
}

Renderable::Renderable()
    : mParentNode(NULL), mRenderCategory(Default), mDebugging(false)
{
    activeRenderables++;
}

Renderable::~Renderable()
{
    activeRenderables--;
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

void Renderable::mouseDoubleClickEvent(QMouseEvent *evt)
{
    emit mouseDoubleClicked(evt);
}

void Renderable::mousePressEvent(QMouseEvent *evt)
{
    emit mousePressed(evt);
}

void Renderable::mouseReleaseEvent(QMouseEvent *evt)
{
    emit mouseReleased(evt);
}

void Renderable::mouseEnterEvent(QMouseEvent *evt)
{
    emit mouseEnter(evt);
}

void Renderable::mouseLeaveEvent(QMouseEvent *evt)
{
    emit mouseLeave(evt);
}

void Renderable::mouseMoveEvent(QMouseEvent *evt)
{
    emit mouseMove(evt);
}

};
