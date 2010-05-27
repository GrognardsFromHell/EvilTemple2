
#include "renderqueue.h"
#include "scene.h"

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
        RenderQueue::Default
    };

    for (int catOrder = 0; catOrder < RenderQueue::Count; ++catOrder) {
        RenderQueue::Category category = renderOrder[catOrder];
        const QList<Renderable*> &renderables = d->renderQueue.queuedObjects(category);
        for (int i = 0; i < renderables.size(); ++i) {
            Renderable *renderable = renderables.at(i);

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
}

int Scene::objectsDrawn() const
{
    return d->objectsDrawn;
}

};