
#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtCore/QMap>

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

class AnimationEvent {
friend QDataStream &operator >>(QDataStream &stream, AnimationEvent &event);
public:

    enum Type {
        Script = 0,
        Action = 1,
        Type_ForceDWord = 0x7fffffff
    };

    /**
      * The number of the frame (starting at zero), on which this event occurs.
      */
    uint frame() const;

    /**
      * The type of this event.
      */
    Type type() const;

    /**
      * The content of this event. The meaning of this field depends on the type of event.
      * For script events for instance, this is the scripting code that should be executed.
      */
    const QString &content() const;

private:
    uint mFrame;
    Type mType;
    QString mContent;

};

inline const QString &AnimationEvent::content() const
{
    return mContent;
}

inline AnimationEvent::Type AnimationEvent::type() const
{
    return mType;
}

/**
  Contains the keyframe data for an animated bone.
  */
class AnimationBone {
friend QDataStream &operator >>(QDataStream &stream, AnimationBone &bone);
public:
	const QMap<ushort, Quaternion> &rotationStream() const;

	const QMap<ushort, Vector4> &scaleStream() const;

	const QMap<ushort, Vector4> &translationStream() const;

private:
    QMap<ushort, Quaternion> mRotationStream;
    QMap<ushort, Vector4> mScaleStream;
    QMap<ushort, Vector4> mTranslationStream;
};

inline const QMap<ushort, Quaternion> &AnimationBone::rotationStream() const
{
	return mRotationStream;
}

inline const QMap<ushort, Vector4> &AnimationBone::scaleStream() const
{
	return mScaleStream;
}

inline const QMap<ushort, Vector4> &AnimationBone::translationStream() const
{
	return mTranslationStream;
}

/**
    Models a single animation, which is modeled as a collection of animated bones.
*/
class Animation {
    friend QDataStream &operator >>(QDataStream &stream, Animation &event);
public:

    Animation()
    {
    }

    enum DriveType {
        Time = 0,
        Distance,
        Rotation,
        DriveType_ForceDWord = 0x7fffffff
    };

    const QString &name() const;

    uint frames() const;

    float frameRate() const;

    float dps() const;

    bool isLoopable() const;

    DriveType driveType() const;

    const QVector<AnimationEvent> &events() const;

    const QMap<uint, AnimationBone> &animationBones() const;

private:
    QString mName;
    uint mFrames;
    float mFrameRate;
    float mDps;
    bool mLoopable;
    DriveType mDriveType;
    QVector<AnimationEvent> mEvents;
    QMap<uint, AnimationBone> mAnimationBones;
    Q_DISABLE_COPY(Animation);
};

inline const QString &Animation::name() const
{
    return mName;
}

inline uint Animation::frames() const
{
    return mFrames;
}

inline float Animation::frameRate() const
{
    return mFrameRate;
}

inline float Animation::dps() const
{
    return mDps;
}

inline bool Animation::isLoopable() const
{
    return mLoopable;
}

inline Animation::DriveType Animation::driveType() const
{
    return mDriveType;
}

inline const QVector<AnimationEvent> &Animation::events() const
{
    return mEvents;
}

inline const QMap<uint, AnimationBone> &Animation::animationBones() const
{
    return mAnimationBones;
}

/**
A bone for skeletal animation
*/
class Bone {
    friend class Model;
public:

    Bone() : mParent(NULL)
    {
    }

    /**
     * Returns the name of this bone.
     */
    const QString &name() const;

	/**
		Id of this bone.
      */
	uint boneId() const;

    /**
     * Returns the parent of this bone. NULL if this bone has no parent.
     */
    const Bone *parent() const;

    /**
     * Returns a full transform matrix that will transform a vertex into this bone's space and back into
     * object space with this bones default transformation applied.
     */
    const Matrix4 &defaultTransform() const;

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

    void setName(const QString &name);

    void setParent(const Bone *bone);

    void setDefaultTransform(const Matrix4 &defaultTransform);

    void setFullWorldInverse(const Matrix4 &fullWorldInverse);

    void setRelativeWorld(const Matrix4 &relativeWorld);

private:
	uint mBoneId;
    QString mName;
    const Bone *mParent; // Undeletable ref to parent
    Matrix4 mDefaultTransform;
    Matrix4 mFullWorldInverse;
    Matrix4 mRelativeWorld;
};

inline uint Bone::boneId() const
{
	return mBoneId;
}

inline const QString &Bone::name() const
{
    return mName;
}

inline const Bone *Bone::parent() const
{
    return mParent;
}

inline const Matrix4 &Bone::defaultTransform() const
{
    return mDefaultTransform;
}

inline const Matrix4 &Bone::fullWorldInverse() const
{
    return mFullWorldInverse;
}

inline const Matrix4 &Bone::relativeWorld() const
{
    return mRelativeWorld;
}

inline void Bone::setBoneId(uint id)
{
	mBoneId = id;
}

inline void Bone::setName(const QString &name)
{
    mName = name;
}

inline void Bone::setParent(const Bone *bone)
{
    mParent = bone;
}

inline void Bone::setDefaultTransform(const Matrix4 &defaultTransform)
{
    mDefaultTransform = defaultTransform;
}

inline void Bone::setFullWorldInverse(const Matrix4 &fullWorldInverse)
{
    mFullWorldInverse = fullWorldInverse;
}

inline void Bone::setRelativeWorld(const Matrix4 &relativeWorld)
{
    mRelativeWorld = relativeWorld;
}

class Model {
public:
    Model();
    ~Model();

    bool open(const QString &filename, const RenderStates &renderState);
    void close();

    const Vector4 *positions;
    const Vector4 *normals;
    const BoneAttachment *attachments; // This CAN be null!
    const float *texCoords;
    int vertices;

    GLuint positionBuffer;
    GLuint normalBuffer;
    GLuint texcoordBuffer;

    int faces;
    QScopedArrayPointer<FaceGroup> faceGroups;

    void drawNormals() const;

    const QString &error() const;

    /**
     * Returns the bones of the skeleton in this model. If it has no skeleton, the list is empty.
     */
    const QVector<Bone> &bones() const;

    /**
      * Returns an animation by name. NULL if no such animation is found.
      */
    const Animation *animation(const QString &name) const;

private:
    typedef QHash<QString, const Animation*> AnimationMap;

    AnimationMap mAnimationMap;

    QScopedArrayPointer<Animation> mAnimations;

    QVector<Bone> mBones;

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

inline const Animation *Model::animation(const QString &name) const
{
    AnimationMap::const_iterator it = mAnimationMap.find(name);

    if (it == mAnimationMap.end()) {
        return NULL;
    } else {
        return it.value();
    }
}

inline const QString &Model::error() const
{
    return mError;
}

inline const QVector<Bone> &Model::bones() const
{
	return mBones;
}

typedef QSharedPointer<Model> SharedModel;

}

#endif
