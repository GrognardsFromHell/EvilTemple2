#ifndef MODELWRITER_H
#define MODELWRITER_H

#include <QDataStream>
#include <QHash>
#include "troika_model.h"
#include "materialconverter.h"

class ModelWriter
{
public:
    ModelWriter(const QString &filename, QDataStream &stream);

    void writeSkeleton(const Troika::Skeleton *skeleton);
    void writeBindingPose(const Troika::MeshModel *model);
    void writeTextures(const QList<HashedData> &textures);
    void writeMaterials(const QList<HashedData> &materialScripts, const QStringList &placeholders);
        void writeMaterialReferences(const QStringList &materialFiles);
    void writeChunk(uint chunk, bool required, const QByteArray &data);
    void writeVertices(const QVector<Troika::Vertex> &vertices);
    void writeFaces(const QList<QSharedPointer<Troika::FaceGroup> > &faceGroups, const QHash<QString,int> &materialMapping);
    void writeBoneAttachments(const QVector<Troika::Vertex> &vertices);
    void writeBoundingVolumes(const Troika::MeshModel *model);
    void writeAnimations(const Troika::MeshModel *model);
    void writeAnimationAliases(const QHash<QString,QString> &aliases);

    void finish(); // Writes CRC values and finishes the overall file structure

    enum ChunkTypes {
        Textures = 1,
        Materials = 2,
        MaterialReferences = 3,
        Geometry = 4,
        Faces = 5,
        Skeleton = 6, // Skeletal data
        BindingPose = 7, // Assigns vertices to bones and gives the full inverse matrices used for it
        BoundingVolumes = 8, // Bounding volumes,
        Animations = 9, // Animations
        AnimationAliases = 10, // Additional animation aliases
        Metadata = 0xFFFF,  // Last chunk is always metadata
        UserChunk = 0x10000, // This gives plenty of room. 16-bit are reserved for application chunks
    };

private:
    void startChunk(uint chunk, bool required);
    void finishChunk();

    uint lastChunkStart;
    QDataStream &stream;
    uint chunks;
    QString mFilename;
};

#endif // MODELWRITER_H
