
#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtCore/QScopedArrayPointer>
#include <QtCore/QMap>
#include <QtCore/QDataStream>

#include "materialstate.h"
#include "renderstates.h"
#include "util.h"
#include "vertexbufferobject.h"

#include "animation.h"
#include "skeleton.h"

#include <cmath>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

    class Materials;

    /**
      Represents a group of faces, and the material used to draw them.
      */
    class FaceGroup {
    public:
        FaceGroup();

        MaterialState *material;
        int placeholderId; // If this face group is connected to a placeholder
        // slot, this is the index pointing to it, otherwise it's -1

        IndexBufferObject buffer;
        QVector<ushort> indices;
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

        static const uint MaxCount = 4; // Maximum number of attachments

        int mBones[MaxCount]; // Index to every bone this vertex is attached to

        float mWeights[MaxCount]; // Weights for every one of these bones. Assumption is: Sum(mWeights) = 1.0f
    };
    
    class Model : public AlignedAllocation
    {
    public:
        Model();
        ~Model();

        bool load(const QString &filename,
                  Materials *materials,
                  const RenderStates &renderState);

        Vector4 *positions;
        Vector4 *normals;
        const BoneAttachment *attachments; // This CAN be null!
        const float *texCoords;
        int vertices;

        VertexBufferObject positionBuffer;
        VertexBufferObject normalBuffer;
        VertexBufferObject texcoordBuffer;

        int faces;
        QScopedArrayPointer<FaceGroup> faceGroups;

        void drawNormals() const;

        float radius() const;
        float radiusSquared() const;
        const Box3d &boundingBox() const;

        const QString &error() const;

        const Skeleton *skeleton() const;

        /**
         * Returns an animation by name. NULL if no such animation is found.
         */
        const Animation *animation(const QString &name) const;

        QStringList animations() const;

        /**
          Checks if the model supports the given animation.
          */
        bool hasAnimation(const QString &name) const;

        const QStringList &placeholders() const;

        /**
          Indicates that this model needs its normals recalculated after it was animated.
          */
        bool needsNormalsRecalculated() const;

    private:
        typedef QHash<QString, const Animation*> AnimationMap;

        typedef QScopedPointer<char, AlignedDeleter> AlignedPointer;

        AnimationMap mAnimationMap;

        QScopedArrayPointer<Animation> mAnimations;

        QScopedArrayPointer<MaterialState> materialState;

        QStringList mPlaceholders;

        bool mNeedsNormalsRecalculated;

        Skeleton *mSkeleton;

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

    inline bool Model::needsNormalsRecalculated() const
    {
        return mNeedsNormalsRecalculated;
    }

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

    inline const Skeleton *Model::skeleton() const
    {
        return mSkeleton;
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

    inline bool Model::hasAnimation(const QString &name) const
    {
        return mAnimationMap.contains(name);
    }

    inline const QString &Model::error() const
    {
        return mError;
    }

    typedef QSharedPointer<Model> SharedModel;

    uint getActiveModels();

}

#endif
