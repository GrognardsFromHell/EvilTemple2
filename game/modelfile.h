
#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include <QtCore/QString>

#include "materialstate.h"
#include "renderstates.h"
#include "util.h"

using namespace GameMath;

namespace EvilTemple {

struct FaceGroup {
	MaterialState *material;
	uint elementCount;
	GLuint buffer;
	
	FaceGroup();
	~FaceGroup();
};

/**
  Models the attachment of a single vertex to several bones.
  This is used for skeletal animation
  */
class BoneAttachment {
public:

    int count() const {
        return mBoneCount;
    }

    const int *bones() const {
        return &mBones[0];
    }

    const float *weights() const {
        return &mWeights[0];
    }

private:
    int mBoneCount; // Number of bones this vertex is attached to

    static const uint MaxCount = 6; // Maximum number of attachments

    /*
     TODO: Can't we convert this to a pointer instead?
         The lifetime of the bones is exactly as long as the lifetime of the model, which in turn
         dictates the lifetime of the attachments. Thus, it should be no problem.
         In addition, we could just point to the full transform matrix instead, further simplifying things.
     */
    int mBones[MaxCount]; // Index to every bone this vertex is attached to

    float mWeights[MaxCount]; // Weights for every one of these bones. Assumption is: Sum(mWeights) = 1.0f
};

/**
  A bone for skeletal animation
  */
class Bone {
public:

    const char *name() const;

    const Matrix4 &defaultPoseTransform() const;

private:
    char mName[60];
    int mParentId;
    Matrix4 mDefaultWorldPose;
};

inline const char *Bone::name() const
{
    return mName;
}

inline const Matrix4 &Bone::defaultPoseTransform() const
{
    return mDefaultWorldPose;
}

class Model {
public:
	Model();
	~Model();

        bool open(const QString &filename, const RenderStates &renderState);
	void close();

	Vector4 *positions;
	Vector4 *normals;
        const BoneAttachment *attachments; // This CAN be null!
        const Bone *bones; // Bones in the model
	float *texCoords;
        int vertices;
        int bonesCount;

	GLuint positionBuffer;
	GLuint normalBuffer;
	GLuint texcoordBuffer;

	int faces;	
        QScopedArrayPointer<FaceGroup> faceGroups;

	void drawNormals() const;

	const QString &error() const;

private:
    QScopedArrayPointer<MaterialState> materialState;

    typedef QScopedPointer<char, AlignedDeleter> AlignedPointer;

    AlignedPointer boneData;
    AlignedPointer boneAttachmentData;
    AlignedPointer vertexData;
    AlignedPointer faceData;
    AlignedPointer textureData;

    void loadVertexData();
    void loadFaceData();

    QString mError;
};

inline const QString &Model::error() const
{
	return mError;
}

}

#endif
