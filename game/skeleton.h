#ifndef SKELETON_H
#define SKELETON_H

#include <QVector>
#include <QObject>
#include <QDataStream>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

/**
    A bone for skeletal animation
  */
class Bone : public AlignedAllocation
{
public:
    Bone();

    /**
     * Returns the name of this bone.
     */
    const QByteArray &name() const;

    /**
        Id of this bone.
    */
    uint boneId() const;

    /**
     * Returns the parent of this bone. NULL if this bone has no parent.
     */
    const Bone *parent() const;

    /**
     * Transforms from object space into this bone's local space, assuming the default pose of the skeleton.
     */
    const Matrix4 &fullWorldInverse() const;

    /**
     * Transforms from this bone's space into the local space of the parent or in case of a bone without a parent
     * into object space.
     */
    const Matrix4 &relativeWorld() const;

    void setBoneId(uint id);

    void setName(const QByteArray &name);

    void setParent(const Bone *bone);

    void setFullWorldInverse(const Matrix4 &fullWorldInverse);

    void setRelativeWorld(const Matrix4 &relativeWorld);

    /**
      Returns the matrix that transforms from this bones space into world space. This is derived from the
      parent's full world matrix and this bone's world matrix.
      */
    const Matrix4 &fullWorld() const;

    /**
      Returns this bone's full transform matrix, which is the full world inverse of the bind pose, multiplied
      with the full world matrix of this bone.
      */
    const Matrix4 &fullTransform() const;

private:
    Matrix4 mFullWorldInverse;
    Matrix4 mRelativeWorld;
    Matrix4 mFullTransform;
    Matrix4 mFullWorld;

    uint mBoneId;
    QByteArray mName;
    const Bone *mParent; // Undeletable ref to parent
};

inline Bone::Bone() : mParent(NULL)
{
}

inline uint Bone::boneId() const
{
    return mBoneId;
}

inline const QByteArray &Bone::name() const
{
    return mName;
}

inline const Bone *Bone::parent() const
{
    return mParent;
}

inline const Matrix4 &Bone::fullWorldInverse() const
{
    return mFullWorldInverse;
}

inline const Matrix4 &Bone::relativeWorld() const
{
    return mRelativeWorld;
}

inline const Matrix4 &Bone::fullWorld() const
{
    return mFullWorld;
}

inline const Matrix4 &Bone::fullTransform() const
{
    return mFullTransform;
}

inline void Bone::setBoneId(uint id)
{
    mBoneId = id;
}

inline void Bone::setName(const QByteArray &name)
{
    mName = name;
}

inline void Bone::setParent(const Bone *bone)
{
    mParent = bone;
}

inline void Bone::setFullWorldInverse(const Matrix4 &fullWorldInverse)
{
    mFullWorldInverse = fullWorldInverse;
}

inline void Bone::setRelativeWorld(const Matrix4 &relativeWorld)
{
    mRelativeWorld = relativeWorld;
}

/**
  A skeleton is used for skeletal animation and for positioning of attached objects on a
  model. It has a name for easier identification and can be copied to animate this skeleton
  for multiple instances of a model.
  */
class Skeleton
{
    friend QDataStream &operator >>(QDataStream &stream, Skeleton &skeleton);
public:

    /**
      Returns the name of this skeleton.
      */
    const QString &name() const;

    /**
      Sets a name for this skeleton. This is used to identify this skeleton in debugging
      messages.
      */
    void setName(const QString &name);

    const Bone *bone(const QByteArray &name) const;

    const QVector<Bone> &bones() const;

private:
    QString mName;
    QVector<Bone> mBones;
    QHash<QByteArray, Bone*> mBoneMap;
};

inline const Bone *Skeleton::bone(const QByteArray &name) const
{
    return mBoneMap.value(name, NULL);
}

inline const QVector<Bone> &Skeleton::bones() const
{
    return mBones;
}

inline const QString &Skeleton::name() const
{
    return mName;
}

inline void Skeleton::setName(const QString &name)
{
    mName = name;
}

QDataStream &operator >>(QDataStream &stream, Skeleton &skeleton);

}

#endif // SKELETON_H
