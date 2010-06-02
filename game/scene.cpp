
#include "renderqueue.h"
#include "scene.h"
#include "lighting.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class SceneData {
public:

    SceneData() : objectsDrawn(0)
    {
    }

    QList<SharedSceneNode> sceneNodes;
    int objectsDrawn;
    RenderQueue renderQueue;
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
}

void Scene::elapseTime(float elapsedSeconds)
{
    for (int i = 0; i < d->sceneNodes.size(); ++i) {
        d->sceneNodes[i]->elapseTime(elapsedSeconds);
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
        RenderQueue::Lights
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

};
