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
Q_OBJECT
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

    bool overrideMaterial(const QString &name, const SharedMaterialState &state);
    bool clearOverrideMaterial(const QString &name);
    void clearOverrideMaterials();

private:

    void animateVertices(const SharedModel &model, Vector4 *transformedPositions, Vector4 *transformedNormals, QGLBuffer *positionBuffer, QGLBuffer *normalBuffer);

    Matrix4 *mFullWorld;
    Matrix4 *mFullTransform;

    SharedModel mModel;

    const Animation *mCurrentAnimation;

    float mPartialFrameTime;
    uint mCurrentFrame;
    bool mCurrentFrameChanged;

    Vector4 *mTransformedPositions;
    Vector4 *mTransformedNormals;

    QList<Vector4*> mTransformedPositionsAddMeshes;
    QList<Vector4*> mTransformedNormalsAddMeshes;

    QList<QGLBuffer*> mPositionBufferAddMeshes;
    QList<QGLBuffer*> mNormalBufferAddMeshes;

    QGLBuffer mPositionBuffer;
    QGLBuffer mNormalBuffer;

    QList<SharedModel> mAddMeshes;

    // These relate to mModel->placeholders()
    QVector<SharedMaterialState> mReplacementMaterials;
    
    Q_DISABLE_COPY(ModelInstance);
};

inline const SharedModel &ModelInstance::model() const
{
    return mModel;
}

}

#endif // MODELINSTANCE_H
