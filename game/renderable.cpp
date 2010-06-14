
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

void LineRenderable::render(RenderStates &renderStates)
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
