#ifndef MODELFILEREADER_H
#define MODELFILEREADER_H

#include <QFile>
#include <QBuffer>
#include <QScopedPointer>
#include <QDataStream>
#include <QString>
#include <QIODevice>
#include <QByteArray>

namespace EvilTemple {

enum ModelChunkType {
    Chunk_Textures = 1,
    Chunk_Materials = 2,
    Chunk_MaterialReferences = 3,
    Chunk_MaterialPlaceholders = 4,
    Chunk_Geometry = 5,
    Chunk_Faces = 6,
    Chunk_Skeleton = 7, // Skeletal data
    Chunk_BindingPose = 8, // Assigns vertices to bones
    Chunk_BoundingVolumes = 9, // Bounding volumes,
    Chunk_Animations = 10, // Animations
    Chunk_AnimationAliases = 11, // Aliases for animations
    Chunk_Metadata = 0xFFFF,  // Last chunk is always metadata
};

/**
  A reader for the model file chunks format.
  */
class ModelFileReader
{
public:
    bool open(const QString &filename);
    bool open(const QByteArray &data);
    bool open(QIODevice *device);
    void close();

    const QString &error() const;

    QDataStream &stream();

    uint version() const;
    uint checksum() const;
    uint chunkCount() const;

    /**
    True if nextChunk can be called.
    */
    bool hasNextChunk() const;

    /**
    Advance to and read the header of the next chunk. True on success, false if
    an error occurred. The reader will guarantee that the chunk types are read in
    ascending order, or this method will fail otherwise.
    */
    bool nextChunk();

    /*
    The following methods return the chunk header of the current chunk. Only valid after nextChunk has been called
    at least once.
    */

    ModelChunkType chunkType() const;
    QString chunkTypeName() const;
    uint chunkSize() const;
    uint chunkFlags() const;
    uint chunkReserved() const;

private:
    bool readHeader();
    bool isStreamError();

    QBuffer mBuffer;
    QFile mFile;
    QString mError;
    QDataStream mStream;

    /*
    Information about the next chunk.
    */
    uint mNextChunk;
    qint64 mNextChunkOffset;

    /*
    File header fields.
    */
    uint mChunkCount;
    uint mVersion;
    uint mChecksum;

    /*
    Chunk header fields.
    */
    ModelChunkType mChunkType;
    uint mChunkSize;
    uint mChunkFlags;
    uint mChunkReserved;
};

inline const QString &ModelFileReader::error() const
{
    return mError;
}

inline QDataStream &ModelFileReader::stream()
{
    return mStream;
}

inline uint ModelFileReader::version() const
{
    return mVersion;
}

inline uint ModelFileReader::checksum() const
{
    return mChecksum;
}

inline ModelChunkType ModelFileReader::chunkType() const
{
    return mChunkType;
}

inline uint ModelFileReader::chunkFlags() const
{
    return mChunkFlags;
}

inline uint ModelFileReader::chunkSize() const
{
    return mChunkSize;
}

inline uint ModelFileReader::chunkReserved() const
{
    return mChunkReserved;
}

inline uint ModelFileReader::chunkCount() const
{
    return mChunkCount;
}

inline bool ModelFileReader::hasNextChunk() const
{
    return mNextChunk < mChunkCount;
}

}

#endif // MODELFILEREADER_H
