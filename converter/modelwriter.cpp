#include "modelwriter.h"

#include "material.h"
using namespace Troika;

static const uint MAGIC = 0x4C444F4D; // MODL
static const uint VERSION = 1;
static const uint RESERVED = 0;

// There will be at most 15 bytes of padding.
static const char PADDING_DATA[15] = { (char)0xFF, };

// Padding for the name of bones
static const char BONE_NAME_PADDING[64] = { 0, };

inline uint getRequiredPadding(uint size) {
    uint result = 16 - (size % 16);

    return result & 0xF;
}

ModelWriter::ModelWriter(QDataStream &_stream) : stream(_stream), chunks(0), lastChunkStart(-1)
{
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Write magic, version and two placeholders for # of chunks and CRC
    stream << MAGIC << VERSION << (uint)0 << (uint)chunks;
}

void ModelWriter::writeChunk(uint chunk, bool required, const QByteArray &data)
{
    uint flags = 0;

    if (required) {
        flags |= 1;
    }

    stream << chunk << flags << (uint)0;

    if (data.isNull()) {
        stream << (uint)0; // We don't want to serialize "null" byte arrays
    } else {
        // If size is not a multiple of 16, add padding
        if ((data.size() % 16) != 0) {
            uint padding = 16 - (data.size() % 16);
            stream << (uint)(data.size() + padding);
            stream.writeRawData(data.data(), data.size());
            stream.writeRawData(PADDING_DATA, padding);
        } else {
            stream << data; // Auto-writes a 32-bit integer size before the data
        }
    }

    chunks++;
}

QMatrix4x4 relativeWorld(const AnimationStream *stream, const Troika::Bone &bone) {
    // Quite annoying: Build the relative matrix, rebuild the entire matrix for the bone + parent
    const AnimationBoneState *state = stream->getBoneState(bone.id);

    if (!state) {
        return bone.relativeWorld;
    } else {
        QMatrix4x4 matrix;
        matrix.setToIdentity();
        matrix.translate(state->translation);
        matrix.rotate(state->rotation);
        matrix.scale(state->scale);

        return matrix;
    }
}

void ModelWriter::writeBones(const Troika::Skeleton *skeleton)
{
    startChunk(Bones, true);

    stream << (uint)skeleton->bones().size() << RESERVED << RESERVED << RESERVED;

    const Animation *animation = skeleton->findAnimation("item_idle");
    AnimationStream *animStream = NULL;
    if (animation)
        animStream = animation->openStream(skeleton);

    foreach (const Troika::Bone &bone, skeleton->bones()) {
        QByteArray name = bone.name.toLatin1();
        Q_ASSERT(name.size() < 60);
        stream.writeRawData(name.data(), name.size());
        stream.writeRawData(BONE_NAME_PADDING, 60 - name.size());

        // Id of parent (or -1)
        stream << (int)bone.parentId;

        // Use the bones of the item_idle animation
        if (animation && animStream) {
            QMatrix4x4 transform = relativeWorld(animStream, bone) * bone.fullWorldInverse;

            int parentId = bone.parentId;
            while (parentId != -1) {
                transform = relativeWorld(animStream, skeleton->bones()[parentId]) * transform;
                parentId = skeleton->bones()[parentId].parentId;
            }

            for (int col = 0; col < 4; ++col) {
                for (int row = 0; row < 4; ++row) {
                    stream << (float)transform(row, col);
                }
            }
        } else {
            for (int col = 0; col < 4; ++col) {
                for (int row = 0; row < 4; ++row) {
                    stream << (float)bone.defaultPoseWorld(row, col);
                }
            }
        }

    }

    finishChunk();
}

void ModelWriter::writeBoneAttachments(const QVector<Troika::Vertex> &vertices)
{
    startChunk(BoneAttachments, true);

    stream << (uint)vertices.size() << RESERVED << RESERVED << RESERVED;

    foreach (const Troika::Vertex &vertex, vertices) {
        stream << (uint)vertex.attachmentCount;
        for (int i = 0; i < 6; ++i) {
            if (i < vertex.attachmentCount) {
                stream << (int)vertex.attachmentBone[i];
            } else {
                stream << (int)-1;
            }
        }
        for (int i = 0; i < 6; ++i) {
            if (i < vertex.attachmentCount) {
                stream << (float)vertex.attachmentWeight[i];
            } else {
                stream << (float)0;
            }
        }
    }

    finishChunk();
}

void ModelWriter::writeTextures(const QList<HashedData> &textures)
{
    startChunk(Textures, true);

    stream << (uint)textures.size() << RESERVED << RESERVED << RESERVED;

    foreach (const HashedData &hashedData, textures) {
       stream << hashedData;
    }

    finishChunk();
}

void ModelWriter::writeMaterials(const QList<HashedData> &materialScripts)
{
    startChunk(Materials, true);

    stream << (uint)materialScripts.size() << RESERVED << RESERVED << RESERVED;

    foreach (const HashedData &hashedData, materialScripts) {
        stream << hashedData;
    }

    finishChunk();
}

void ModelWriter::writeVertices(const QVector<Troika::Vertex> &vertices)
{
    startChunk(Geometry, true);

    // Make the header also 16-byte padded.
    stream << (uint)vertices.size() << RESERVED << RESERVED << RESERVED;

    // Write position data
    foreach (const Troika::Vertex &vertex, vertices) {
        stream << vertex.positionX << vertex.positionY << vertex.positionZ << (float)1;
    }

    // Write normals
    foreach (const Troika::Vertex &vertex, vertices) {
        stream << vertex.normalX << vertex.normalY << vertex.normalZ << (float)0;
    }

    foreach (const Troika::Vertex &vertex, vertices) {
        stream << vertex.texCoordX << vertex.texCoordY;
    }

    finishChunk();
}

void ModelWriter::writeFaces(const QList<QSharedPointer<Troika::FaceGroup> > &faceGroups, const QHash<QString,int> &materialMapping)
{
    startChunk(Faces, true);

    stream << (uint)faceGroups.size() << RESERVED << RESERVED << RESERVED;

    foreach (const QSharedPointer<Troika::FaceGroup> &faceGroup, faceGroups) {
        int materialId = -1;
        if (faceGroup->material()) {
            materialId = materialMapping[faceGroup->material()->name()];
        }
        uint elementSize = sizeof(quint16);
        stream << materialId << (uint)(faceGroup->faces().size() * 3) << elementSize << RESERVED;
        foreach (const Troika::Face &face, faceGroup->faces()) {
            stream << face.vertices[0] << face.vertices[1] << face.vertices[2];
        }
    }

    finishChunk();
}

void ModelWriter::startChunk(uint chunk, bool required)
{
    Q_ASSERT(lastChunkStart == -1);

    lastChunkStart = stream.device()->pos();

    uint flags = required ? 1 : 0;

    stream << chunk << flags << RESERVED << RESERVED;

    chunks++;
}

void ModelWriter::finishChunk()
{
    Q_ASSERT(lastChunkStart != -1);

    uint endOfChunk = stream.device()->pos();
    uint paddingRequired = getRequiredPadding(endOfChunk);

    if (paddingRequired > 0) {
        endOfChunk += paddingRequired;
        stream.writeRawData(PADDING_DATA, paddingRequired);
    }

    uint sizeOfChunk = endOfChunk - lastChunkStart - 16; // Subtract size of the chunk's header.

    stream.device()->seek(lastChunkStart + 12);

    stream << sizeOfChunk;

    stream.device()->seek(endOfChunk);

    lastChunkStart = -1;
}

void ModelWriter::finish()
{
    Q_ASSERT(!stream.device()->isSequential());

    uint checksum = 0;

    stream.device()->seek(8);
    stream << checksum << chunks;
}
