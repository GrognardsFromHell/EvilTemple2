
#ifndef SCENE_H
#define SCENE_H

#include <QObject>
#include <QtCore/QScopedPointer>

#include "scenenode.h"

namespace EvilTemple {

class SceneData;

/**
  Models a scene that can be drawn by the engine.
  */
class Scene : public QObject {
Q_OBJECT
public:    
    Scene();
    ~Scene();

    void elapseTime(float elapsedSeconds);

    void render(RenderStates &renderStates);

public slots:
    void addNode(const SharedSceneNode &node);

    void clear();

    void addTextOverlay(const Vector4 &position, const QString &text, const QColor &color, float lifetime = 2.5f);

    /**
    Returns the number of objects drawn by the last call to render.
    */
    int objectsDrawn() const;

    SharedSceneNode pickNode(const Ray3d &ray) const;

    SharedRenderable pickRenderable(const Ray3d &ray) const;

private:
    QScopedPointer<SceneData> d;
    Q_DISABLE_COPY(Scene)
};

}

Q_DECLARE_METATYPE(EvilTemple::Scene*)

#endif // SCENE_H
