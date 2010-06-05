
#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtCore/QMap>
#include <QtCore/QDataStream>

#include "materialstate.h"
#include "renderstates.h"
#include "util.h"

#include <cmath>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

    struct FaceGroup {
        MaterialState *material;
        int placeholderId; // If this face group is connected to a placeholder 
        // slot, this is the index pointing to it, otherwise it's -1
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

    template<typename T> inline T lerp(const T &a, const T &b, float t)
    {
        Vector4 diff = b - a;
        return a + t * diff;
    }

    template<> inline Quaternion lerp<Quaternion>(const Quaternion &a, const Quaternion &b, float t)
    {
        float opposite;
        float inverse;
        float dot = a.dot(b);
        bool flag = false;

        if( dot < 0.0f )
        {
            flag = true;
            dot = -dot;
        }

        if( dot > 0.999999f )
        {
            inverse = 1.0f - t;
            opposite = flag ? -t : t;
        }
        else
        {
            float acos = static_cast<float>( std::acos( static_cast<double>( dot ) ) );
            float invSin = static_cast<float>( ( 1.0f / sin( static_cast<double>( acos ) ) ) );

            inverse = ( static_cast<float>( sin( static_cast<double>( (1.0f - t) * acos ) ) ) ) * invSin;
            opposite = flag ? ( ( static_cast<float>( -sin( static_cast<double>( t * acos ) ) ) ) * invSin ) : ( ( static_cast<float>( sin( static_cast<double>( t * acos ) ) ) ) * invSin );
        }

        return inverse * a + opposite * b;

	// TODO: Try to use SLERP here, but also account for direction like this NLERP implementation
        /*float dot = a.dot(b);
	Quaternion result;

	if (dot >= 0) {
		result = (1 - t) * a + t * b;
	} else {
		result = (1 - t) * a - t * b;
        }

        result.normalize();
        return result;*/
    }

    template<typename T, typename FT = ushort> class KeyframeStream
    {
        template<typename _T, typename _FT>
        friend inline QDataStream &operator >>(QDataStream &stream, KeyframeStream<_T,_FT> &keyframeStream);
    public:
	KeyframeStream() : mFrameStream(0), mValueStream(0), mSize(0)
	{
	}

	~KeyframeStream()
	{
            delete [] mFrameStream;
            delete [] mValueStream;
	}

        inline T interpolate(FT frame, FT totalFrames) const
	{
            Q_ASSERT(mSize > 0);

            if (mSize == 1)
                return mValueStream[0];

            for (int i = 0; i < mSize; ++i) {
                FT keyFrame = mFrameStream[i];

                if (keyFrame == frame) {
                    return mValueStream[i];
                } else if (keyFrame > frame) {
                    // We've reached the "latter" frame. We asumme here, that
                    // this CANNOT be the first keyframe, since that MUST be frame 0
                    Q_ASSERT(i > 0);
                    FT prevKeyFrame = mFrameStream[i - 1];
                    Q_ASSERT(keyFrame > prevKeyFrame); // Otherwise we have duplicate frames.
                    float delta = (frame - prevKeyFrame) / (float)(keyFrame - prevKeyFrame);

                    return lerp<T>(mValueStream[i - 1], mValueStream[i], delta);
                }
            }

            /*
			return mValueStream[mSize - 1];
			No visual difference was perceived between clamping (the line above) and interpolating
			back to the starting frame. So we interpolate back to the starting frame if we're behind
			the last keyframe.
		*/

            // Interpolate between the last and first frame (TODO: is this really a good idea?)
            FT lastKeyFrame = mFrameStream[mSize - 1];
            float delta = (frame - lastKeyFrame) / (float)(totalFrames - lastKeyFrame);
            return lerp<T>(mValueStream[mSize - 1], mValueStream[0], delta);
	}	

    private:
	FT mSize;
	FT* mFrameStream;
	T* mValueStream;

	Q_DISABLE_COPY(KeyframeStream);
    };

    template<typename T, typename FT>
    inline QDataStream &operator >>(QDataStream &stream, KeyframeStream<T,FT> &keyframeStream)
    {
	uint size;
	stream >> size;
	keyframeStream.mSize = size;
	keyframeStream.mFrameStream = new FT[size];
	keyframeStream.mValueStream = new T[size];

	for (int i = 0; i < size; ++i) {
            stream >> keyframeStream.mFrameStream[i] >> keyframeStream.mValueStream[i];
	}

	return stream;
    }

    /**
        Contains the keyframe data for an animated bone.
    */
    class AnimationBone
    {
        friend QDataStream &operator >>(QDataStream &stream, AnimationBone &bone);
    public:
	AnimationBone()
	{
	}

	Matrix4 getTransform(ushort frame, ushort totalFrames) const;
	
    private:
	KeyframeStream<Quaternion> rotationStream;
	KeyframeStream<Vector4> scaleStream;
	KeyframeStream<Vector4> translationStream;

	Q_DISABLE_COPY(AnimationBone);
    };

    inline Matrix4 AnimationBone::getTransform(ushort frame, ushort totalFrames) const
    {
	Quaternion rotation = rotationStream.interpolate(frame, totalFrames);
	Vector4 scale = scaleStream.interpolate(frame, totalFrames);
	Vector4 translation = translationStream.interpolate(frame, totalFrames);

	return Matrix4::transformation(scale, rotation, translation);
    }

    /**
    Models a single animation, which is modeled as a collection of animated bones.
*/
    class Animation
    {
        friend QDataStream &operator >>(QDataStream &stream, Animation &event);
    public:

        Animation() : mAnimationBones(0)
        {
        }

        ~Animation()
        {
        }

        enum DriveType {
            Time = 0,
            Distance,
            Rotation,
            DriveType_ForceDWord = 0x7fffffff
                               };
	
        /**
     Type of the container that maps bone ids to their respective animated state.
     */
        typedef QHash<uint, const AnimationBone*> BoneMap;

        const QString &name() const;

        uint frames() const;

        float frameRate() const;

        float dps() const;

        bool isLoopable() const;

        DriveType driveType() const;

        const QVector<AnimationEvent> &events() const;

        const BoneMap &animationBones() const;

    private:
        QString mName;
        uint mFrames;
        float mFrameRate;
        float mDps;
        bool mLoopable;
        DriveType mDriveType;
        QVector<AnimationEvent> mEvents;
        BoneMap mAnimationBonesMap;
        AnimationBone *mAnimationBones;
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

    inline const Animation::BoneMap &Animation::animationBones() const
    {
	return mAnimationBonesMap;
    }

    /**
A bone for skeletal animation
*/
    class Bone
    {
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

    class Model
    {
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

        float radius() const;
        float radiusSquared() const;
        const Box3d &boundingBox() const;

        const QString &error() const;

        /**
         * Returns the bones of the skeleton in this model. If it has no skeleton, the list is empty.
         */
        const QVector<Bone> &bones() const;

        /**
         * Returns an animation by name. NULL if no such animation is found.
         */
        const Animation *animation(const QString &name) const;

        const QStringList &placeholders() const;

    private:
        typedef QHash<QString, const Animation*> AnimationMap;

        typedef QScopedPointer<char, AlignedDeleter> AlignedPointer;

        AnimationMap mAnimationMap;

        QScopedArrayPointer<Animation> mAnimations;

        QVector<Bone> mBones;

        QScopedArrayPointer<MaterialState> materialState;

        QStringList mPlaceholders;

        AlignedPointer boneData;
        AlignedPointer boneAttachmentData;
        AlignedPointer vertexData;
        AlignedPointer faceData;
        AlignedPointer textureData;

        void loadVertexData();
        void loadFaceData();

        float mRadius;
        float mRadiusSquared;
        Box3d mBoundingBox;

        QString mError;
    };

    inline const QStringList &Model::placeholders() const
    {
        return mPlaceholders;
    }

    inline float Model::radius() const
    {
        return mRadius;
    }

    inline float Model::radiusSquared() const
    {
        return mRadiusSquared;
    }

    inline const Box3d &Model::boundingBox() const
    {
        return mBoundingBox;
    }

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
