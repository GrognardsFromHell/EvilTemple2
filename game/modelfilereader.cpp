#include "modelfilereader.h"

namespace EvilTemple {

bool ModelFileReader::open(const QString &filename)
{
    mFile.setFileName(filename);

    if (!mFile.open(QIODevice::ReadOnly)) {
        mError = mFile.errorString();
        return false;
    }

    return open(&mFile);
}

bool ModelFileReader::open(const QByteArray &data)
{
    mBuffer.setData(data);
    return open(&mBuffer);
}

bool ModelFileReader::open(QIODevice *device)
{
    mStream.setDevice(device);
    mStream.setByteOrder(QDataStream::LittleEndian);
    mStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    return readHeader();
}

void ModelFileReader::close()
{
    mStream.unsetDevice();
    mFile.close();
    mBuffer.close();
}

bool ModelFileReader::readHeader()
{
    // Check file magic.
    char magic[4];
    mStream.readRawData(magic, sizeof(magic));
    if (mStream.status() != QDataStream::Ok || strncmp(magic, "MODL", sizeof(magic))) {
        mError = "File has invalid file header.";
        return false;
    }

    mStream >> mVersion >> mChecksum >> mChunkCount;

    mNextChunk = 0;
    mNextChunkOffset = mStream.device()->pos();

    return mStream.status() == QDataStream::Ok;
}

inline bool ModelFileReader::isStreamError()
{
    switch (mStream.status()) {
    case QDataStream::ReadCorruptData:
        mError = "Read corrupted data.";
        return true;
    case QDataStream::ReadPastEnd:
        mError = "Read past end of file.";
        return true;
    case QDataStream::Ok:
        return false;
    default:
        mError = "Unknown stream state.";
        return true;
    }
}

bool ModelFileReader::nextChunk()
{
    if (!hasNextChunk()) {
        mError = "Trying to read a chunk, although all chunks have already been read. Use hasNextChunk.";
        return false;
    }

    if (mStream.device()->pos() < mNextChunkOffset) {
        mStream.device()->seek(mNextChunkOffset);
    } else if (mStream.device()->pos() > mNextChunkOffset) {
        qDebug("More than the data of chunk %s has been consumed. Seeking back to the next chunk.",
            qPrintable(chunkTypeName()));
        mStream.device()->seek(mNextChunkOffset);
    }

    // Read the chunk header
    uint type;

    mStream >> type >> mChunkFlags >> mChunkReserved >> mChunkSize;

    if (isStreamError())
        return false;

    if (mNextChunk != 0 && type <= mChunkType) {
        mError = QString("Read chunk %1 after chunk %2. Chunks need to be in ascending order.")
            .arg(type).arg(mChunkType);
        //return false;
    }

    mChunkType = (ModelChunkType)type;

    mNextChunkOffset = mStream.device()->pos() + mChunkSize;
    mNextChunk++;

    return true;
}

QString ModelFileReader::chunkTypeName() const
{
    switch (mChunkType) {
    case Chunk_Textures:
        return "Textures";
    case Chunk_Materials:
        return "Materials";
    case Chunk_MaterialReferences:
        return "MaterialReferences";
    case Chunk_MaterialPlaceholders:
        return "MaterialPlaceholders";
    case Chunk_Geometry:
        return "Geometry";
    case Chunk_Faces:
        return "Faces";
    case Chunk_Skeleton:
        return "Skeleton";
    case Chunk_BindingPose:
        return "BindingPose";
    case Chunk_BoundingVolumes:
        return "BoundingVolumes";
    case Chunk_Animations:
        return "Animations";
    case Chunk_AnimationAliases:
        return "AnimationAliases";
    case Chunk_Metadata:
        return "Metadata";
    default:
        return QString("UnknownType%1").arg((uint)mChunkType);
    }
}

}
