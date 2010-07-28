
#include "modelfilechunks.h"

// Define this to disable the little-endian reading optimizations below
// #define NO_LITTLE_ENDIAN_OPTIMIZATION

namespace EvilTemple {

QDataStream &operator >>(QDataStream &stream, ModelGeometry &chunk)
{
    uint count;

    stream >> count;
    stream.skipRawData(sizeof(uint) * 3); // Reserved

    chunk.mPositions.resize(count);
    chunk.mNormals.resize(count);
    chunk.mUvCoordinates.resize(count);

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN && !defined(NO_LITTLE_ENDIAN_OPTIMIZATION)
    stream.readRawData(reinterpret_cast<char*>(chunk.mPositions.data()), count * sizeof(Vector4));
    stream.readRawData(reinterpret_cast<char*>(chunk.mNormals.data()), count * sizeof(Vector4));
    stream.readRawData(reinterpret_cast<char*>(chunk.mUvCoordinates.data()), count * sizeof(QVector2D));
#else
    float x, y, z, w;

    for (uint i = 0; i < count; ++i) {
        stream >> x >> y >> z >> w;
        chunk.mPositions[i] = Vector4(x, y, z, w);
    }

    for (uint i = 0; i < count; ++i) {
        stream >> x >> y >> z >> w;
        chunk.mNormals[i] = Vector4(x, y, z, w);
    }

    for (uint i = 0; i < count; ++i) {
        stream >> x >> y;
        chunk.mUvCoordinates[i].setX(x);
        chunk.mUvCoordinates[i].setY(y);
    }
#endif

    return stream;
}

QDataStream &operator >>(QDataStream &stream, ModelFaceGroup &faceGroup)
{
    int materialId;
    uint elementCount, elementSize;

    stream >> materialId >> elementCount >> elementSize;
    stream.skipRawData(sizeof(uint)); // Reserved

    if (materialId < 0) {
        faceGroup.mMaterialId = -1;
        faceGroup.mMaterialPlaceholderId = (- materialId) - 1;
    } else {
        faceGroup.mMaterialId = materialId;
        faceGroup.mMaterialPlaceholderId = -1;
    }

    Q_ASSERT(elementSize == sizeof(ushort)); // Only shorts supported at the moment.

    faceGroup.mIndices.resize(elementCount);

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN && !defined(NO_LITTLE_ENDIAN_OPTIMIZATION)
    stream.readRawData(reinterpret_cast<char*>(faceGroup.mIndices.data()), elementCount * elementSize);
#else
    for (uint i = 0; i < elementCount; ++i)
        stream >> faceGroup.mIndices[i];
#endif

    return stream;
}

enum
{
    FacesFlag_RecalculateNormals = 1
};

QDataStream &operator >>(QDataStream &stream, ModelFaces &faces)
{
    uint count, flags;
    stream >> count >> flags;
    stream.skipRawData(sizeof(uint) * 2); // Reserved

    faces.mNeedsRecalculatedNormals = ((flags & FacesFlag_RecalculateNormals) != 0);

    faces.mFaceGroups.resize(count);

    for (uint i = 0; i < count; ++i)
        stream >> faces.mFaceGroups[i];

    return stream;
}

}
