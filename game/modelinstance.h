#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtOpenGL/QGLBuffer>

#include "modelfile.h"
#include "materialstate.h"

namespace EvilTemple {

class RenderStates;

/**
    A model instance manages the per-instance state of models. This includes animation state
    and transformed position and normal data.
  */
class ModelInstance
{
public:
    ModelInstance();
	~ModelInstance();

    void setModel(const SharedModel &model);

    void elapseTime(float elapsedSeconds);

    void drawNormals() const;
    void draw(const RenderStates &renderStates) const;
    void draw(const RenderStates &renderStates, MaterialState *overrideMaterial) const;

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

    Q_DISABLE_COPY(ModelInstance);
};

}

#endif // MODELINSTANCE_H
