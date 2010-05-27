
#ifndef SCENE_H
#define SCENE_H

#include <QtCore/QScopedPointer>

#include "scenenode.h"

namespace EvilTemple {

class SceneData;

/**
  Models a scene that can be drawn by the engine.
  */
class Scene {
public:
    Scene();
    ~Scene();

    void addNode(const SharedSceneNode &node);

    void elapseTime(float elapsedSeconds);

    void render(RenderStates &renderStates);

    /**
    Returns the number of objects drawn by the last call to render.
    */
    int objectsDrawn() const;

private:
    QScopedPointer<SceneData> d;
};

}

#endif // SCENE_H
