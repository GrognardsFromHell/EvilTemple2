
#ifndef MODEL_H
#define MODEL_H

#include "gameglobal.h"

#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtCore/QScopedArrayPointer>
#include <QtCore/QMap>
#include <QtCore/QDataStream>

#include "materialstate.h"
#include "renderstates.h"
#include "vertexbufferobject.h"

#include "animation.h"
#include "skeleton.h"
#include "bindingpose.h"

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

    class GAME_EXPORT Model : public AlignedAllocation
    {
    public:
        Model();
        ~Model();

        bool load(const QString &filename,
            Materials *materials,
            const RenderStates &renderState);

        Vector4 *positions;
        Vector4 *normals;
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

        const BindingPose *bindingPose() const;

        /**
         * Returns an animation by name. NULL if no such animation is found.
         */
        const Animation *animation(const QByteArray &name) const;

        QList<QByteArray> animations() const;

        /**
          Checks if the model supports the given animation.
          */
        bool hasAnimation(const QByteArray &name) const;

        const QVector<QByteArray> &placeholders() const;

        /**
          Indicates that this model needs its normals recalculated after it was animated.
          */
        bool needsNormalsRecalculated() const;

    private:
        typedef QHash<QByteArray, const Animation*> AnimationMap;

        typedef QScopedPointer<char, AlignedDeleter> AlignedPointer;

        AnimationMap mAnimationMap;

        QScopedArrayPointer<Animation> mAnimations;

        QVector<SharedMaterialState> mMaterialState;

        QVector<QByteArray> mPlaceholders;

        bool mNeedsNormalsRecalculated;

        Skeleton *mSkeleton;

        BindingPose *mBindingPose;

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

    inline const QVector<QByteArray> &Model::placeholders() const
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

    inline const BindingPose *Model::bindingPose() const
    {
        return mBindingPose;
    }

    inline const Animation *Model::animation(const QByteArray &name) const
    {
        AnimationMap::const_iterator it = mAnimationMap.find(name);

        if (it == mAnimationMap.end()) {
            return NULL;
        } else {
            return it.value();
        }
    }

    inline bool Model::hasAnimation(const QByteArray &name) const
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
