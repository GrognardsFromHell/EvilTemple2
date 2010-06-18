
#include <QFont>
#include <QFontMetrics>
#include <QPainterPath>

#include "renderqueue.h"
#include "scene.h"
#include "lighting.h"
#include "renderable.h"
#include "scenenode.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

static const float FadeoutTime = 0.5f; // in seconds

struct TextOverlay {
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

    SceneData() : objectsDrawn(0)
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

    QList<SharedSceneNode> sceneNodes;
    int objectsDrawn;
    RenderQueue renderQueue;
    QList<TextOverlay*> activeOverlays;
    QFont font;
    QPainter textPainter;
    int textureWidth, textureHeight;
};

Scene::Scene() : d(new SceneData)
{
}

Scene::~Scene()
{
}

void Scene::addNode(const SharedSceneNode &node)
{
    d->sceneNodes.append(node);
    node->setScene(this);
}

void Scene::removeNode(const SharedSceneNode &node)
{
    d->sceneNodes.removeAll(node);
    node->setScene(NULL);
}

void Scene::elapseTime(float elapsedSeconds)
{
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
    d->objectsDrawn = 0;

    d->renderQueue.clear();

    // Build a view frustum
    Frustum viewFrustum;
    viewFrustum.extract(renderStates.viewProjectionMatrix());

    for (int i = 0; i < d->sceneNodes.size(); ++i) {
        d->sceneNodes[i]->addVisibleObjects(viewFrustum, &d->renderQueue);
    }

    const RenderQueue::Category renderOrder[RenderQueue::Count] = { 
        RenderQueue::ClippingGeometry,
        RenderQueue::Default,
        RenderQueue::Lights,
        RenderQueue::DebugOverlay
    };

    // Find all light sources that are visible.
    QList<const Light*> visibleLights;

    foreach (Renderable *lightCanidate, d->renderQueue.queuedObjects(RenderQueue::Lights)) {
        Light *light = qobject_cast<Light*>(lightCanidate);
        if (light) {
            visibleLights.append(light);
        }
    }

    for (int catOrder = 0; catOrder < RenderQueue::Count; ++catOrder) {
        RenderQueue::Category category = renderOrder[catOrder];
        const QList<Renderable*> &renderables = d->renderQueue.queuedObjects(category);
        for (int i = 0; i < renderables.size(); ++i) {            
            Renderable *renderable = renderables.at(i);

            // Find all light sources that intersect the bounding volume of the given object
            QList<const Light*> activeLights;

            for (int j = 0; j < visibleLights.size(); ++j) {
                // TODO: This ignores the full position
                float squaredDistance = (visibleLights[j]->position() - renderable->parentNode()->position()).lengthSquared();
                if (squaredDistance < visibleLights[j]->range() * visibleLights[j]->range())
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
}

int Scene::objectsDrawn() const
{
    return d->objectsDrawn;
}

SharedSceneNode Scene::pickNode(const Ray3d &ray) const
{
    SharedSceneNode picked;

    for (int i = 0; i < d->sceneNodes.size(); ++i) {
        const SharedSceneNode &node = d->sceneNodes.at(i);

        if (!node->isInteractive())
            continue;

        Ray3d localRay = node->fullTransform().inverted() * ray;

        if (localRay.intersects(node->boundingBox())) {
            picked = node;
        }
    }

    return picked;
}

SharedRenderable Scene::pickRenderable(const Ray3d &ray) const
{
    SharedRenderable picked;
    float distance = std::numeric_limits<float>::infinity();

    for (int i = 0; i < d->sceneNodes.size(); ++i) {
        const SharedSceneNode &node = d->sceneNodes.at(i);

        if (!node->isInteractive())
            continue;

        Ray3d localRay = node->fullTransform().inverted() * ray;

        if (localRay.intersects(node->boundingBox())) {
            foreach (const SharedRenderable &renderable, node->attachedObjects()) {
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
    // Disconnect signals for everything
    foreach (const SharedSceneNode &node, d->sceneNodes) {
        foreach (const SharedRenderable &renderable, node->attachedObjects()) {
            renderable->disconnect();
        }
    }

    d->sceneNodes.clear();
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
