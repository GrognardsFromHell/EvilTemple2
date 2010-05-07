#ifndef MODELWRITER_H
#define MODELWRITER_H

#include <QDataStream>
#include "model.h"

class ModelWriter
{
public:
    ModelWriter(QDataStream &stream);

    void writeChunk(uint chunk, bool required, const QByteArray &data);
    void writeMaterials(const QList<Troika::Material> &materials);
    void writeVertices(const QVector<Troika::Vertex> &vertices);
    void writeFaces(const QList<QSharedPointer<Troika::FaceGroup> > &faceGroups);

    void finish(); // Writes CRC values and finishes the overall file structure

    enum ChunkTypes {
        Materials = 1,
        Geometry = 2,
        Faces = 3,
        Bones = 4, // Skeletal data
        BoneAttachments = 5, // Assigns vertices to bones
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
