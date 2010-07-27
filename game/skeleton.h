#ifndef SKELETON_H
#define SKELETON_H

#include <QVector>
#include <QByteArray>
#include <QHash>
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
friend class Animation;
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
    Bone *parent();

    /**
     * Returns the parent of this bone. NULL if this bone has no parent.
     */
    const Bone *parent() const;

    /**
     * Transforms from this bone's space into the local space of the parent or in case of a bone without a parent
     * into object space.
     */
    const Matrix4 &relativeWorld() const;

    void setBoneId(uint id);

    void setName(const QByteArray &name);

    void setParent(Bone *bone);

    void setRelativeWorld(const Matrix4 &relativeWorld);

    /**
     * Sets the full world matrix of this bone.
     */
    void setFullWorld(const Matrix4 &fullWorld);

    /**
      Returns the matrix that transforms from this bones space into world space. This is derived from the
      parent's full world matrix and this bone's world matrix.
      */
    const Matrix4 &fullWorld() const;

private:
    Matrix4 mRelativeWorld;
    Matrix4 mFullWorld;

    uint mBoneId;
    QByteArray mName;
    Bone *mParent; // Undeletable ref to parent
};

inline Bone::Bone() : mParent(NULL)
{
}

inline uint Bone::boneId() const
{
    return mBoneId;
}

inline void Bone::setFullWorld(const Matrix4 &fullWorld)
{
    mFullWorld = fullWorld;
}

inline const QByteArray &Bone::name() const
{
    return mName;
}

inline const Bone *Bone::parent() const
{
    return mParent;
}

inline Bone *Bone::parent()
{
    return mParent;
}

inline const Matrix4 &Bone::relativeWorld() const
{
    return mRelativeWorld;
}

inline const Matrix4 &Bone::fullWorld() const
{
    return mFullWorld;
}

inline void Bone::setBoneId(uint id)
{
    mBoneId = id;
}

inline void Bone::setName(const QByteArray &name)
{
    mName = name;
}

inline void Bone::setParent(Bone *bone)
{
    mParent = bone;
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
    friend class Animation;
public:

    Skeleton();
    ~Skeleton();

    Skeleton(const Skeleton &other);

    typedef QVector<const Bone*> ConstBones;

    typedef QVector<Bone*> Bones;

    /**
      Returns the name of this skeleton.
      */
    const QString &name() const;

    /**
      Sets a name for this skeleton. This is used to identify this skeleton in debugging
      messages.
      */
    void setName(const QString &name);

    /**
     * Retrieves a bone by name from the skeleton.
     */
    Bone *bone(const QByteArray &name);

    /**
     * Retrieves a bone by name from the skeleton.
     */
    const Bone *bone(const QByteArray &name) const;

    /**
     * Returns a bone with the given id if it exists.
     */
    Bone *bone(uint boneId);

    /**
     * Returns a non-mutable bone with the given id if it exists.
     */
    const Bone *bone(uint boneId) const;

    /**
      * Returns a vector of mutable pointers to the bones of this skeleton.
      */
    const Bones &bones();

    /**
      * Returns a vector of non-mutable pointers to the bones of this skeleton.
      */
    const ConstBones &bones() const;

private:
    QString mName;
    Bone *mBones;
    Bones mBonePointers;
    mutable ConstBones mConstBonePointers;
    QHash<QByteArray, Bone*> mBoneMap;
};

inline Skeleton::Skeleton()
{
}

inline Bone *Skeleton::bone(const QByteArray &name)
{
    return mBoneMap.value(name, NULL);
}

inline const Bone *Skeleton::bone(const QByteArray &name) const
{
    return mBoneMap.value(name, NULL);
}

inline const QString &Skeleton::name() const
{
    return mName;
}

inline void Skeleton::setName(const QString &name)
{
    mName = name;
}

inline const Skeleton::Bones &Skeleton::bones()
{
    return mBonePointers;
}

inline const Skeleton::ConstBones &Skeleton::bones() const
{
    if (mConstBonePointers.isEmpty()) {
        mConstBonePointers.resize(mBonePointers.size());
        for (int i = 0; i < mBonePointers.size(); ++i) {
            mConstBonePointers[i] = mBonePointers[i];
        }
    }
    return mConstBonePointers;
}

inline const Bone *Skeleton::bone(uint boneId) const
{
    if (int(boneId) < mBonePointers.size())
        return mBonePointers.at(boneId);
    else
        return NULL;
}

inline Bone *Skeleton::bone(uint boneId)
{
    if (int(boneId) < mBonePointers.size())
        return mBonePointers.at(boneId);
    else
        return NULL;
}

QDataStream &operator >>(QDataStream &stream, Skeleton &skeleton);

}

#endif // SKELETON_H
