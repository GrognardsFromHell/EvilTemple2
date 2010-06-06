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

    void setIdleAnimation(const QString &idleAnimation);
    const QString &idleAnimation() const;

    bool isIdling() const; // No animation is playing

    bool playAnimation(const QString &name, bool loop = false);

signals:
    void animationFinished(const QString &name, bool canceled);

private:

    void animateVertices(const SharedModel &model, Vector4 *transformedPositions, Vector4 *transformedNormals, QGLBuffer *positionBuffer, QGLBuffer *normalBuffer,
                         QVector<uint> *boneMapping);

    void playIdleAnimation();

    Matrix4 *mFullWorld;
    Matrix4 *mFullTransform;

    SharedModel mModel;

    QString mIdleAnimation;
    bool mIdling; // mCurrentAnimation is the idle animation
    bool mLooping; // Only relevant if not idling (idle is always looped)
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

    // When an add-mesh is loaded, this maps the bone-ids from the addmesh to the bone ids in the
    // skeleton used by mModel
    QList< QVector<uint> > mAddMeshBoneMapping;

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

inline bool ModelInstance::isIdling() const
{
    return mIdling;
}

}

#endif // MODELINSTANCE_H
