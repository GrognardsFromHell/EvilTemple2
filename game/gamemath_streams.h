#ifndef GAMEMATH_STREAMS_H
#define GAMEMATH_STREAMS_H

#include <gamemath.h>
#include <QDataStream>

inline QDataStream &operator >>(QDataStream &stream, GameMath::Matrix4 &matrix) {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        stream.readRawData(reinterpret_cast<char*>(matrix.data()), sizeof(float) * 16);
#else
        for (int i = 0; i < 16; ++i) {
            stream >> matrix.data()[i];
        }
#endif
        return stream;
}

inline QDataStream &operator <<(QDataStream &stream, GameMath::Vector4 &vector) {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    stream.writeRawData(reinterpret_cast<char*>(vector.data()), sizeof(float) * 4);
#else
    stream << vector.data()[0] << vector.data()[1] << vector.data()[2] << vector.data()[3];
#endif
    return stream;
}

inline QDataStream &operator >>(QDataStream &stream, GameMath::Vector4 &vector) {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    stream.readRawData(reinterpret_cast<char*>(vector.data()), sizeof(float) * 4);
#else
    stream >> vector.data()[0] >> vector.data()[1] >> vector.data()[2] >> vector.data()[3];
#endif
    return stream;
}

inline QDataStream &operator >>(QDataStream &stream, GameMath::Quaternion &quaternion) {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    stream.readRawData(reinterpret_cast<char*>(quaternion.data()), sizeof(float) * 4);
#else
    stream >> quaternion.data()[0] >> quaternion.data()[1] >> quaternion.data()[2] >> quaternion.data()[3];
#endif
    return stream;
}

inline QDataStream &operator >>(QDataStream &stream, GameMath::Box3d &aabb)
{
    GameMath::Vector4 minCorner, maxCorner;
    stream >> minCorner >> maxCorner;
    aabb.setMinimum(minCorner);
    aabb.setMaximum(maxCorner);
    return stream;
}

#endif // GAMEMATH_STREAMS_H
