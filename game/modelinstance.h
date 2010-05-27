#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtOpenGL/QGLBuffer>

#include "modelfile.h"
#include "materialstate.h"
#include "renderable.h"

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class RenderStates;

struct AddMesh {
    QString boneName;
    SharedModel model;
};

/**
    A model instance manages the per-instance state of models. This includes animation state
    and transformed position and normal data.
  */
class ModelInstance : public Renderable
{
public:
    ModelInstance();
	~ModelInstance();

    void setModel(const SharedModel &model);

    void elapseTime(float elapsedSeconds);

    void drawNormals() const;
    void render(RenderStates &renderStates);
    void draw(const RenderStates &renderStates, MaterialState *overrideMaterial) const;

    const SharedModel &model() const;

    Matrix4 getBoneSpace(const QString &boneName) const;

    void addMesh(const SharedModel &model);

    IntersectionResult intersect(const Ray3d &ray) const;

    const Box3d &boundingBox();

    const Matrix4 &worldTransform() const;

private:
	Matrix4 *mFullWorld;
	Matrix4 *mFullTransform;

    SharedModel mModel;

    const Animation *mCurrentAnimation;

	float mPartialFrameTime;
	uint mCurrentFrame;

	Vector4 *mTransformedPositions;
	Vector4 *mTransformedNormals;

    QGLBuffer mPositionBuffer;
    QGLBuffer mNormalBuffer;

    QList<SharedModel> mAddMeshes;
    
    Q_DISABLE_COPY(ModelInstance);
};

inline const SharedModel &ModelInstance::model() const
{
    return mModel;
}

}

#endif // MODELINSTANCE_H
