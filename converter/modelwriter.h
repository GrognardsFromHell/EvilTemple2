#ifndef MODELWRITER_H
#define MODELWRITER_H

#include <QDataStream>
#include <QHash>
#include "model.h"

class ModelWriter
{
public:
    ModelWriter(QDataStream &stream);

    void writeTextures(const QList<QByteArray> &textures);
    void writeMaterials(const QList<QByteArray> &materialScripts);
    void writeChunk(uint chunk, bool required, const QByteArray &data);
    void writeVertices(const QVector<Troika::Vertex> &vertices);
    void writeFaces(const QList<QSharedPointer<Troika::FaceGroup> > &faceGroups, const QHash<QString,int> &materialMapping);

    void finish(); // Writes CRC values and finishes the overall file structure

    enum ChunkTypes {
        Textures = 1,
        Materials = 2,
        Geometry = 3,
        Faces = 4,
        Bones = 5, // Skeletal data
        BoneAttachments = 6, // Assigns vertices to bones
        Metadata = 0xFFFF,  // Last chunk is always metadata
        UserChunk = 0x10000, // This gives plenty of room. 16-bit are reserved for application chunks
    };

private:
    void startChunk(uint chunk, bool required);
    void finishChunk();

    uint lastChunkStart;
    QDataStream &stream;
    uint chunks;
};

#endif // MODELWRITER_H
