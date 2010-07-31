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
    void writeMaterials(const QList<HashedData> &materialScripts);
    void writeMaterialPlaceholders(const QVector<QByteArray> &placeholders);
    void writeMaterialReferences(const QStringList &materialFiles);
    void writeChunk(uint chunk, bool required, const QByteArray &data);
    void writeVertices(const QVector<Troika::Vertex> &vertices);
    void writeFaces(const QList<QSharedPointer<Troika::FaceGroup> > &faceGroups, const QHash<QString,int> &materialMapping);
    void writeBoneAttachments(const QVector<Troika::Vertex> &vertices);
    void writeBoundingVolumes(const Troika::MeshModel *model);
    void writeAnimations(const Troika::MeshModel *model);
    void writeAnimationAliases(const QHash<QByteArray,QByteArray> &aliases);

    void finish(); // Writes CRC values and finishes the overall file structure

    enum ChunkTypes {
        Textures = 1,
        Materials = 2,
        MaterialReferences = 3,
        MaterialPlaceholders = 4,
        Geometry = 5,
        Faces = 6,
        Skeleton = 7, // Skeletal data
        BindingPose = 8, // Assigns vertices to bones and gives the full inverse matrices used for it
        BoundingVolumes = 9, // Bounding volumes,
        Animations = 10, // Animations
        AnimationAliases = 11, // Additional animation aliases
        Metadata = 0xFFFF,  // Last chunk is always metadata
        UserChunk = 0x10000, // This gives plenty of room. 16-bit are reserved for application chunks
    };

private:
    void startChunk(uint chunk, bool required);
    void finishChunk();

    uint chunks;
    uint lastChunkStart;
    QDataStream &stream;
    QString mFilename;
};

#endif // MODELWRITER_H
