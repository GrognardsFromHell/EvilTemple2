
#include <QFont>
#include <QFontMetrics>
#include <QPainterPath>

#include "scriptables.h"

#include "renderqueue.h"
#include "scene.h"
#include "lighting.h"
#include "renderable.h"
#include "scenenode.h"
#include "profiler.h"
#include "materials.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

static const float FadeoutTime = 0.5f; // in seconds

struct TextOverlay : public AlignedAllocation {
    Vector4 position;
    QString text;
    QColor color;
    float lifetime;
    float elapsedTime;
    QImage texture;
    int realWidth, realHeight;
};

class SceneData {
public:

    SceneData(Materials *materials)
        : objectsDrawn(0), behindWallsMaterial(materials->load(":/material/behindwalls_material.xml"))
    {
        font.setFamily("Fontin");
        font.setPointSize(12);
        font.setWeight(QFont::Bold);
        font.setStyleStrategy((QFont::StyleStrategy)(QFont::PreferAntialias|QFont::PreferQuality));
    }

    ~SceneData()
    {
        qDeleteAll(activeOverlays);
    }

    SharedMaterialState behindWallsMaterial;
    QList<SceneNode*> sceneNodes;
    int objectsDrawn;
    RenderQueue renderQueue;
    QList<TextOverlay*> activeOverlays;
    QFont font;
    QPainter textPainter;
    int textureWidth, textureHeight;
};

Scene::Scene(Materials *materials) : d(new SceneData(materials))
{
}

Scene::~Scene()
{
}

SceneNode *Scene::createNode()
{
    SceneNode *node = new SceneNode(this);
    d->sceneNodes.append(node);
    return node;
}

void Scene::removeNode(SceneNode *node)
{
    if (!node)
        return;

    d->sceneNodes.removeOne(node);
    node->setParentNode(NULL);
    node->deleteLater();
}

void Scene::elapseTime(float elapsedSeconds)
{
    ProfileScope<Profiler::SceneElapseTime> profiler;

    for (int i = 0; i < d->sceneNodes.size(); ++i) {
        d->sceneNodes[i]->elapseTime(elapsedSeconds);
    }

    QList<TextOverlay*>::iterator it = d->activeOverlays.begin();
    while (it != d->activeOverlays.end()) {
        (*it)->elapsedTime += elapsedSeconds;
        if ((*it)->elapsedTime >= (*it)->lifetime + FadeoutTime) {
            it = d->activeOverlays.erase(it);
        } else {
            ++it;
        }
    }
}

void Scene::render(RenderStates &renderStates)
{
    ProfileScope<Profiler::SceneRender> profiler;

    d->objectsDrawn = 0;

    d->renderQueue.clear();

    // Build a view frustum
    Frustum viewFrustum;
    viewFrustum.extract(renderStates.viewProjectionMatrix());

    for (int i = 0; i < d->sceneNodes.size(); ++i) {
        d->sceneNodes[i]->addVisibleObjects(viewFrustum, &d->renderQueue);
    }

    const Renderable::Category renderOrder[Renderable::Count] = {
        Renderable::ClippingGeometry,
        Renderable::StaticGeometry,
        Renderable::Default,
        Renderable::Lights,
        Renderable::DebugOverlay
    };

    // Find all light sources that are visible.
    QList<const Light*> visibleLights;

    foreach (Renderable *lightCanidate, d->renderQueue.queuedObjects(Renderable::Lights)) {
        Light *light = qobject_cast<Light*>(lightCanidate);
        if (light) {
            visibleLights.append(light);
        }
    }

    for (int catOrder = 0; catOrder < Renderable::Count; ++catOrder) {
        Renderable::Category category = renderOrder[catOrder];
        const QList<Renderable*> &renderables = d->renderQueue.queuedObjects(category);
        for (int i = 0; i < renderables.size(); ++i) {
            Renderable *renderable = renderables.at(i);

            // Find all light sources that intersect the bounding volume of the given object
            QList<const Light*> activeLights;

            SceneNode *sceneNode = renderable->parentNode();

            float bbExtent = (sceneNode->worldBoundingBox().maximum() - sceneNode->worldBoundingBox().minimum()).length();

            for (int j = 0; j < visibleLights.size(); ++j) {
                // TODO: This ignores the full position
                // Get BB extent:

                float distance = (visibleLights[j]->position() - sceneNode->position()).length();
                if (distance <= visibleLights[j]->range() + bbExtent)
                    activeLights.append(visibleLights[j]);
            }

            renderStates.setActiveLights(activeLights);

            // TODO: Remove this.
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            renderStates.setWorldMatrix(renderable->worldTransform());
            renderable->render(renderStates);

            d->objectsDrawn++;
        }
    }

    renderStates.setActiveLights(QList<const Light*>());

    // Now, render the behind-walls sections
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    const QList<Renderable*> &clippingRenderables = d->renderQueue.queuedObjects(Renderable::ClippingGeometry);

    for (int i = 0; i < clippingRenderables.size(); ++i) {
        Renderable *renderable = clippingRenderables.at(i);

        renderStates.setWorldMatrix(renderable->worldTransform());
        renderable->render(renderStates);

        d->objectsDrawn++;
    }

    glDepthFunc(GL_GEQUAL); // Flip depth-test so primitives are drawn when depth-test fails
    glDepthMask(GL_FALSE); // But don't actually modify the depth-buffer

    const QList<Renderable*> &defaultRenderables = d->renderQueue.queuedObjects(Renderable::Default);

    for (int i = 0; i < defaultRenderables.size(); ++i) {
        ModelInstance *renderable = qobject_cast<ModelInstance*>(defaultRenderables.at(i));
        if (!renderable || !renderable->drawsBehindWalls())
            continue;

        renderStates.setWorldMatrix(renderable->worldTransform());
        renderable->render(renderStates, d->behindWallsMaterial.data());
    }

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render active text overlays
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(renderStates.projectionMatrix().data());

    QGLContext *ctx = const_cast<QGLContext*>(QGLContext::currentContext());

    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);

    glDisable(GL_MULTISAMPLE);

    foreach (TextOverlay *overlay, d->activeOverlays) {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        Vector4 screenPos = renderStates.viewMatrix().mapPosition(overlay->position);

        glTranslatef(floor(screenPos.x() - 0.5f * overlay->realWidth),
                     floor(screenPos.y() - 0.5f * overlay->realHeight),
                     floor(screenPos.z()));

        glEnable(GL_TEXTURE_2D);
        ctx->bindTexture(overlay->texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        float alpha = 1;
        if (overlay->elapsedTime >= overlay->lifetime) {
            alpha = qMax<float>(0, (FadeoutTime - (overlay->elapsedTime - overlay->lifetime)) / FadeoutTime);
        }

        glColor4f(1, 1, 1, alpha);
        glBegin(GL_QUADS);
        glTexCoord2i(0, 0);
        glVertex2i(0, 0);
        glTexCoord2i(1, 0);
        glVertex2i(overlay->texture.width(), 0);
        glTexCoord2i(1, 1);
        glVertex2i(overlay->texture.width(), overlay->texture.height());
        glTexCoord2i(0, 1);
        glVertex2i(0, overlay->texture.height());
        glEnd();
    }

    glEnable(GL_MULTISAMPLE);
}

int Scene::objectsDrawn() const
{
    return d->objectsDrawn;
}

SceneNode *Scene::pickNode(const Ray3d &ray) const
{
    SceneNode *picked;

    for (int i = 0; i < d->sceneNodes.size(); ++i) {
        SceneNode *node = d->sceneNodes.at(i);

        if (!node->isInteractive())
            continue;

        Ray3d localRay = node->fullTransform().inverted() * ray;

        if (localRay.intersects(node->boundingBox())) {
            picked = node;
        }
    }

    return picked;
}

Renderable *Scene::pickRenderable(const Ray3d &ray) const
{
    Renderable *picked = NULL;
    float distance = std::numeric_limits<float>::infinity();

    for (int i = 0; i < d->sceneNodes.size(); ++i) {
        SceneNode *node = d->sceneNodes.at(i);

        if (!node->isInteractive())
            continue;

        Ray3d localRay = node->fullTransform().inverted() * ray;

        if (localRay.intersects(node->boundingBox())) {
            foreach (Renderable *renderable, node->attachedObjects()) {
                IntersectionResult intersection = renderable->intersect(localRay);
                if (intersection.intersects && intersection.distance < distance) {
                    picked = renderable;
                    distance = intersection.distance;
                }
            }
        }
    }

    return picked;
}

void Scene::clear()
{
    d->sceneNodes.clear();

    // TODO: Delete all child objects
    qDeleteAll(children());
}

inline int roundToPowerOfTwo(int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

inline QRect roundToPowerOfTwo(const QRect &rect)
{
    return QRect(0, 0, roundToPowerOfTwo(rect.width()), roundToPowerOfTwo(rect.height()));
}

void Scene::addTextOverlay(const Vector4 &position, const QString &text, const Vector4 &colorVec, float lifetime)
{
    QColor color(colorVec.x() * 255, colorVec.y() * 255, colorVec.z() * 255, colorVec.w() * 255);

    TextOverlay *textOverlay = new TextOverlay;
    d->activeOverlays.append(textOverlay);

    textOverlay->lifetime = lifetime;
    textOverlay->elapsedTime = 0;
    textOverlay->position = position;
    // Is the following really necessary?
    textOverlay->text = text;
    textOverlay->color = color;

    QFontMetrics fontMetrics(d->font);

    // Measure the size of the texture and resize the pixmap accordingly
    QRect boundingRect = fontMetrics.boundingRect(text);
    textOverlay->realHeight = boundingRect.height();
    textOverlay->realWidth = boundingRect.width();

    QRect textureSize = roundToPowerOfTwo(boundingRect);

    textOverlay->texture = QImage(textureSize.width(), textureSize.height(), QImage::Format_ARGB32);
    textOverlay->texture.fill(0);

    QPainterPath path;
    path.addText(0, boundingRect.height(), d->font, text);

    d->textPainter.begin(&textOverlay->texture);
    d->textPainter.setRenderHint(QPainter::Antialiasing);

    QPen pen(QColor(0,0,0));
    pen.setWidthF(0.15);
    d->textPainter.setPen(pen);
    d->textPainter.setBrush(color);
    d->textPainter.drawPath(path);
    d->textPainter.end();

    /*qDebug("Showing %s @ %f,%f,%f (%d,%d)", qPrintable(text),
           position.x(), position.y(), position.z(),
           boundingRect.width(), boundingRect.height());*/
}

};
