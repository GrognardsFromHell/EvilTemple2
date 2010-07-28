#ifndef MODELFILECHUNKS_H
#define MODELFILECHUNKS_H

#include <QDataStream>
#include <QVector2D>
#include <QVector>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

    /**
      This class represents the static geometry of a model, as read from the model file.
      */
    class ModelGeometry {
    friend QDataStream &operator >>(QDataStream &stream, ModelGeometry &chunk);
    public:
        const QVector<Vector4> &positions() const;
        const QVector<Vector4> &normals() const;
        const QVector<QVector2D> &uvCoordinates() const;

        QVector<Vector4> &positions();
        QVector<Vector4> &normals();
        QVector<QVector2D> &uvCoordinates();
    private:
        QVector<Vector4> mPositions;
        QVector<Vector4> mNormals;
        QVector<QVector2D> mUvCoordinates;
    };

    inline const QVector<Vector4> &ModelGeometry::positions() const
    {
        return mPositions;
    }

    inline const QVector<Vector4> &ModelGeometry::normals() const
    {
        return mNormals;
    }

    inline const QVector<QVector2D> &ModelGeometry::uvCoordinates() const
    {
        return mUvCoordinates;
    }

    inline QVector<Vector4> &ModelGeometry::positions()
    {
        return mPositions;
    }

    inline QVector<Vector4> &ModelGeometry::normals()
    {
        return mNormals;
    }

    inline QVector<QVector2D> &ModelGeometry::uvCoordinates()
    {
        return mUvCoordinates;
    }

    /**
      Represents a set of faces that share the same material properties.
      */
    class ModelFaceGroup {
    friend QDataStream &operator >>(QDataStream &stream, ModelFaceGroup &faceGroup);
    public:
        int materialId() const;
        void setMaterialId(int materialId);

        int materialPlaceholderId() const;
        void setMaterialPlaceholderId(int materialPlaceholderId);

        const QVector<ushort> &indices() const;
        QVector<ushort> &indices();
    private:
        int mMaterialId;
        int mMaterialPlaceholderId;
        QVector<ushort> mIndices;
    };

    inline int ModelFaceGroup::materialId() const
    {
        return mMaterialId;
    }

    inline void ModelFaceGroup::setMaterialId(int materialId)
    {
        mMaterialId = materialId;
    }

    inline int ModelFaceGroup::materialPlaceholderId() const
    {
        return mMaterialPlaceholderId;
    }

    inline void ModelFaceGroup::setMaterialPlaceholderId(int materialPlaceholderId)
    {
        mMaterialPlaceholderId = materialPlaceholderId;
    }

    inline const QVector<ushort> &ModelFaceGroup::indices() const
    {
        return mIndices;
    }

    inline QVector<ushort> &ModelFaceGroup::indices()
    {
        return mIndices;
    }

    /**
      This class represents the triangles produced through a models vertices.
      */
    class ModelFaces {
        friend QDataStream &operator >>(QDataStream &stream, ModelFaces &faces);
    public:
        bool needsRecalculatedNormals() const;

        void setNeedsRecalculatedNormals(bool enabled);

        const QVector<ModelFaceGroup> &faceGroups() const;

        QVector<ModelFaceGroup> &faceGroups();

    private:
        bool mNeedsRecalculatedNormals;
        QVector<ModelFaceGroup> mFaceGroups;
    };

    inline bool ModelFaces::needsRecalculatedNormals() const
    {
        return mNeedsRecalculatedNormals;
    }

    inline void ModelFaces::setNeedsRecalculatedNormals(bool enabled)
    {
        mNeedsRecalculatedNormals = enabled;
    }

    inline const QVector<ModelFaceGroup> &ModelFaces::faceGroups() const
    {
        return mFaceGroups;
    }

    inline QVector<ModelFaceGroup> &ModelFaces::faceGroups()
    {
        return mFaceGroups;
    }

}

#endif // MODELFILECHUNKS_H
