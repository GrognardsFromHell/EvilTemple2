
#define GAMEMATH_NO_MEMORY_OPERATORS
#include <gamemath.h>
using namespace GameMath;

#include "modelwriter.h"

#include "troika_material.h"
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

void ModelWriter::writeBones(const Troika::Skeleton *skeleton)
{
    startChunk(Bones, true);

    stream << (uint)skeleton->bones().size();

    foreach (const Troika::Bone &bone, skeleton->bones()) {
        stream << bone.name.toUtf8();

        // Id of parent (or -1)
        stream << (int)bone.parentId;

        // This is the full world inverse for the bone
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                stream << (float)bone.fullWorldInverse(row, col);
            }
        }

        // This is the default relative world matrix
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                stream << (float)bone.relativeWorld(row, col);
            }
        }

    }

    finishChunk();
}

static QHash<QString,uint> eventTypes;

void printEventTypes() {
    foreach (QString type, eventTypes.keys()) {
        qDebug("%s: %d", qPrintable(type), (uint)eventTypes[type]);
    }
}

// We convert the streams to non-interleaved data, which makes it easier to read them into vectors
struct Streams {
    QMap<uint, QQuaternion> rotationFrames;
    QMap<uint, QVector3D> scaleFrames;
    QMap<uint, QVector3D> translationFrames;

    void appendCurrentState(const AnimationBoneState *state) {
        rotationFrames[state->rotationFrame] = state->rotation;
        scaleFrames[state->scaleFrame] = state->scale;
        translationFrames[state->translationFrame] = state->translation;
    }

    void appendNextState(const AnimationBoneState *state) {
        rotationFrames[state->nextRotationFrame] = state->nextRotation;
        scaleFrames[state->nextScaleFrame] = state->nextScale;
        translationFrames[state->nextTranslationFrame] = state->nextTranslation;
    }
};

void ModelWriter::writeAnimations(const Troika::MeshModel *model)
{
    const Troika::Skeleton *skeleton = model->skeleton();

    startChunk(Animations, true);

    stream << (uint)skeleton->animations().size() << RESERVED << RESERVED << RESERVED;

    QMap<uint, QString> animDataStartMap;

    foreach (const Troika::Animation &animation, skeleton->animations()) {

        if (animDataStartMap.contains(animation.keyFramesDataStart())) {
            // qWarning("%s reuses %s.", qPrintable(animation.name()), qPrintable(animDataStartMap[animation.keyFramesDataStart()]));
            continue;
        }

        animDataStartMap[animation.keyFramesDataStart()] = animation.name();

        // How do we ensure padding if the animations have variable length names?
        stream << animation.name().toUtf8();
        stream << (uint)animation.frames() << animation.frameRate() << animation.dps()
                << (uint)animation.driveType() << animation.loopable() << (uint)animation.events().size();

        foreach (const Troika::AnimationEvent &event, animation.events()) {
            uint frameId = event.frameId;
            stream << frameId;

            if (event.type == "action")
                stream << (uint)1;
            else if (event.type == "script")
                stream << (uint)0;
            else
                qFatal("Unknown event type: %s.", qPrintable(event.type));

            stream << event.action.toUtf8();
        }

        AnimationStream *animStream = animation.openStream(skeleton);

        QMap<uint,Streams> streams;

        // Write out the state of the first bones
        for (int i = 0; i < skeleton->bones().size(); ++i) {
            const AnimationBoneState *boneState = animStream->getBoneState(i);

            if (boneState) {
                streams[i].appendCurrentState(boneState);
            }
        }
        int nextFrame = animStream->getNextFrameId();
        while (!animStream->atEnd()) {
            animStream->readNextFrame();
            if (animStream->getNextFrameId() <= nextFrame && !animStream->atEnd()) {
                _CrtDbgBreak();
            }
            nextFrame = animStream->getNextFrameId();

            for (int i = 0; i < skeleton->bones().size(); ++i) {
                const AnimationBoneState *boneState = animStream->getBoneState(i);

                if (boneState) {
                    streams[i].appendCurrentState(boneState);
                }
            }
        }

        // Also append the state of the last frame
        for (int i = 0; i < skeleton->bones().size(); ++i) {
            const AnimationBoneState *boneState = animStream->getBoneState(i);

            if (boneState) {
                streams[i].appendNextState(boneState);
            }
        }

        animation.freeStream(animStream);

        // Write out the number of bones affected by the animation
        stream << (uint)streams.size();

        // At this point, we have the entire keyframe stream
        // IMPORTANT NOTE: Due to the use of QMap as the container, the keys will be guaranteed to be in ascending order for both bones and frames!
        foreach (uint boneId, streams.keys()) {
            // Write the keyframe streams for every bone
            const Streams &boneStreams = streams[boneId];
            stream << boneId << (uint)boneStreams.rotationFrames.size();
            foreach (uint frameId, boneStreams.rotationFrames.keys()) {
                const QQuaternion &rotation = boneStreams.rotationFrames[frameId];
                stream << (quint16)frameId << rotation.x() << rotation.y() << rotation.z() << rotation.scalar();
            }
            stream << (uint)boneStreams.scaleFrames.size();
            foreach (uint frameId, boneStreams.scaleFrames.keys()) {
                const QVector3D &scale = boneStreams.scaleFrames[frameId];
                stream << (quint16)frameId << scale.x() << scale.y() << scale.z() << (float)0;
            }
            stream << (uint)boneStreams.translationFrames.size();
            foreach (uint frameId, boneStreams.translationFrames.keys()) {
                const QVector3D &translation = boneStreams.translationFrames[frameId];
                stream << (quint16)frameId << translation.x() << translation.y() << translation.z() << (float)0;
            }
        }
    }

    finishChunk();
}

void ModelWriter::writeMaterialReferences(const QStringList &materials)
{
    startChunk(MaterialReferences, true);

    stream << materials;

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

void ModelWriter::writeMaterials(const QList<HashedData> &materialScripts, const QStringList &placeholders)
{
    startChunk(Materials, true);

    stream << (uint)materialScripts.size() << (int)placeholders.size() << RESERVED << RESERVED;

    foreach (const HashedData &hashedData, materialScripts) {
        stream << hashedData;
    }

    foreach (const QString &placeholder, placeholders) {
        stream << placeholder.toUtf8();
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

void ModelWriter::writeBoundingVolumes(const Troika::MeshModel *model)
{
    startChunk(BoundingVolumes, true);

    const QBox3D &box = model->boundingBox();

    // First comes the AABB of the default pose
    stream << box.minimum().x() << box.minimum().y() << box.minimum().z() << (float)1
            << box.maximum().x() << box.maximum().y() << box.maximum().z() << (float)1;

    // In addition, find the maximum distance of a vertex from the origin for this model and use that as the bounding sphere
    float lengthSquared = 0.0f;

    foreach (const Troika::Vertex &vertex, model->vertices()) {
        float distance = vertex.position().lengthSquared();
        if (distance > lengthSquared) {
            lengthSquared = distance;
        }
    }

    // Take sqrt
    float length = std::sqrt(lengthSquared);

    stream << length << lengthSquared;

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
