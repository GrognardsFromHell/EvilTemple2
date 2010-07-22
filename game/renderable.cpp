
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
    emit mouseDoubleClicked(evt->button(), evt->buttons());
}

void Renderable::mousePressEvent(QMouseEvent *evt)
{
    emit mousePressed(evt->button(), evt->buttons());
}

void Renderable::mouseReleaseEvent(QMouseEvent *evt)
{
    emit mouseReleased(evt->button(), evt->buttons());
}

void Renderable::mouseEnterEvent(QMouseEvent *evt)
{
    emit mouseEnter(evt->buttons());
}

void Renderable::mouseLeaveEvent(QMouseEvent *evt)
{
    emit mouseLeave(evt->buttons());
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

};
