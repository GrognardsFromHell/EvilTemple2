#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtOpenGL/QGLBuffer>

#include "modelfile.h"
#include "materialstate.h"

namespace EvilTemple {

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

    void draw() const;
    void draw(MaterialState *overrideMaterial) const;

private:
	struct BoneState {
		Matrix4 fullWorld;
		Matrix4 fullTransform;
	};

	QVector<BoneState> mBoneStates;

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
