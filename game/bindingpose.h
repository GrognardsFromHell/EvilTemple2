#ifndef BINDINGPOSE_H
#define BINDINGPOSE_H

#include <QVector>
#include <QDataStream>
#include <QString>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

/**
    Models the attachment of a single vertex to several bones.
    This is used for skeletal animation
*/
class BoneAttachment {
friend QDataStream &operator >>(QDataStream&, BoneAttachment&);
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

    static const uint MaxCount = 4; // Maximum number of attachments

    int mBones[MaxCount]; // Index to every bone this vertex is attached to

    float mWeights[MaxCount]; // Weights for every one of these bones. Assumption is: Sum(mWeights) = 1.0f
};

/**
  Describes the binding pose of a mesh.

  The name is somewhat misleading since this includes the following information:
  - The vertex binding (vertex->bone association including weight)
  - The bone names and full world inverse matrix used to bind the vertices.
  */
class BindingPose
{
friend QDataStream &operator >>(QDataStream&, BindingPose&);
public:
    const Matrix4 &fullWorldInverse(uint boneId) const;
    const QByteArray &boneName(uint boneId) const;
    const BoneAttachment &attachment(uint vertexId) const;
    uint boneCount() const;
    uint attachmentsCount() const;

private:
    QVector<QByteArray> mBoneNames;
    QVector<Matrix4> mFullWorldInverseMatrices;
    QVector<BoneAttachment> mAttachments;
};

inline const Matrix4 &BindingPose::fullWorldInverse(uint boneId) const
{
    return mFullWorldInverseMatrices[boneId];
}

inline const QByteArray &BindingPose::boneName(uint boneId) const
{
    return mBoneNames[boneId];
}

inline const BoneAttachment &BindingPose::attachment(uint vertexId) const
{
    return mAttachments[vertexId];
}

inline uint BindingPose::boneCount() const
{
    return mBoneNames.size();
}

inline uint BindingPose::attachmentsCount() const
{
    return mAttachments.size();
}

}

#endif // BINDINGPOSE_H
