#include "modelwriter.h"

static const uint MAGIC = 0x4C444F4D; // MODL
static const uint VERSION = 1;
static const uint RESERVED = 0;

// There will be at most 15 bytes of padding.
static const char PADDING_DATA[15] = { 0xFF, };

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

void ModelWriter::writeMaterials(const QList<Troika::Material> &materials)
{
    if (materials.size() == 0) {
        return;
    }

    startChunk(Materials, true);

    stream << (uint)materials.size() << RESERVED << RESERVED << RESERVED;

    foreach (const Troika::Material &material, materials) {

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

void ModelWriter::writeFaces(const QList<QSharedPointer<Troika::FaceGroup> > &faceGroups)
{
    startChunk(Faces, true);

    stream << (uint)faceGroups.size() << RESERVED << RESERVED << RESERVED;

    foreach (const QSharedPointer<Troika::FaceGroup> &faceGroup, faceGroups) {
        uint materialId = 0;
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
