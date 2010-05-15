
#include <QtCore/QFile>
#include <QtGui/QImage>

#include "modelfile.h"
#include "util.h"

namespace EvilTemple {

    Model::Model()
	: faceGroups(0), positions(0), normals(0), texCoords(0), vertices(0), vertexData(0), faceData(0)
        , positionBuffer(0), normalBuffer(0), texcoordBuffer(0), materialState(0), textureData(0), faces(0),
        attachments(0), bonesCount(0), bones(0)
    {
    }

    Model::~Model()
    {
    }

    struct ModelHeader
    {
	char magic[4];
	uint version;
	uint checksum;
	uint chunks;
    };

    struct ChunkHeader
    {
	uint type;
	uint flags;
	uint reserved;
	uint size;
    };

    class ModelTextureSource : public TextureSource {
    public:
        ModelTextureSource(const QVector<Md5Hash> &md5Hashes, const QVector<unsigned char*> &textures,
                           const QVector<int> &textureSizes)
                               : mTextures(textures), mTextureSizes(textureSizes), mMd5Hashes(md5Hashes)
	{
	}

	SharedTexture loadTexture(const QString &name)
	{
            if (name.startsWith('#')) {
                bool ok;
                uint textureId = name.right(name.length() - 1).toUInt(&ok);
                if (ok && textureId < mTextures.size()) {
                    // Check if there already is a texture in the cache
                    SharedTexture texture = GlobalTextureCache::instance().get(mMd5Hashes[textureId]);

                    if (!texture) {
                        QByteArray textureData = QByteArray::fromRawData((char*)mTextures[textureId],
                                                                         mTextureSizes[textureId]);

                        // Try loading the TGA image
                        QImage img;
                        img.loadFromData(textureData, "TGA");

                        texture = SharedTexture(new Texture);
                        texture->load(img);

                        GlobalTextureCache::instance().insert(mMd5Hashes[textureId], texture);
                    }

                    return texture;
                }
            }

            // Create an error texture and return it...
            return SharedTexture(0);
	}

    private:
        QVector<Md5Hash> mMd5Hashes;
	QVector<unsigned char*> mTextures;
	QVector<int> mTextureSizes;
    };

    bool Model::open(const QString &filename, const RenderStates &renderState)
    {
	mError.clear();

        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly)) {
            mError.append(QString("Unable to open model file %1.").arg(filename));
            return false;
	}

        ModelHeader header;

        // TODO: Fix this up for non little-endian systems
        if (!file.read(reinterpret_cast<char*>(&header), sizeof(header))) {
            mError.append(QString("Unable to read model file header from %1.").arg(filename));
            return false;
	}

	if (header.magic[0] != 'M' || header.magic[1] != 'O' || header.magic[2] != 'D' || header.magic[3] != 'L') {
            mError.append(QString("File has invalid magic number: %1.").arg(filename));
            return false;
	}

        QVector<Md5Hash> hashes;
	QVector<unsigned char*> textures;
	QVector<int> texturesSize;

	for (uint i = 0; i < header.chunks; ++i) {
            ChunkHeader chunkHeader;

            if (!file.read(reinterpret_cast<char*>(&chunkHeader), sizeof(ChunkHeader))) {
                mError.append(QString("Unable to read chunk %1 from file %2.").arg(i).arg(filename));
                return false;
            }

            if (chunkHeader.type < 1 || chunkHeader.type > 6) {
                // Skip, unknown chunk
                mError.append(QString("WARN: Unknown chunk type %1 in model file %2.").arg(chunkHeader.type).arg(filename));
                file.seek(file.pos() + chunkHeader.size);
                continue;
            }

            AlignedPointer chunkData((char*)ALIGNED_MALLOC(chunkHeader.size));

            if (!file.read(chunkData.data(), chunkHeader.size)) {
                mError.append(QString("Unable to read data of chunk %1 in %2.").arg(i).arg(filename));
                return false;
            }

            if (chunkHeader.type == 1) {
                textureData.reset(chunkData.take());

                unsigned int textureCount = *(unsigned int*)textureData.data();
                textures.resize(textureCount);
                texturesSize.resize(textureCount);
                hashes.resize(textureCount);

                unsigned char* ptr = reinterpret_cast<unsigned char*>(textureData.data());
                ptr += 16;

                for (int j = 0; j < textureCount; ++j) {
                    memcpy(hashes.data() + j, ptr, sizeof(Md5Hash));
                    ptr += 16;

                    unsigned int size = *(unsigned int*)(ptr);
                    ptr += sizeof(unsigned int);
                    textures[j] = ptr;
                    texturesSize[j] = size;
                    ptr += size;
                }
            } else if (chunkHeader.type == 2) {
                unsigned int count = *reinterpret_cast<unsigned int*>(chunkData.data());
                materialState.reset(new MaterialState[count]);

                char *ptr = chunkData.data() + 16;

                ModelTextureSource textureSource(hashes, textures, texturesSize);

                for (int j = 0; j < count; ++j) {
                    ptr += 16; // Skip the material md5 hash for now
                    unsigned int size = *(unsigned int*)ptr;
                    ptr += sizeof(unsigned int);
                    QByteArray rawMaterialData = QByteArray::fromRawData(ptr, size);
                    ptr += size;

                    Material material;

                    if (!material.loadFromData(rawMaterialData)) {
                        mError.append(QString("Unable to read material from model %1:\n%2").arg(filename)
                                      .arg(material.error()));
                        return false;
                    }

                    if (!materialState[j].createFrom(material, renderState, &textureSource)) {
                        mError.append(QString("Unable to create material state for model %1:\n%2").arg(filename)
                                      .arg(materialState[j].error()));
                        return false;
                    }
                }

            } else if (chunkHeader.type == 3) {
                vertexData.reset(chunkData.take());
                loadVertexData();
            } else if (chunkHeader.type == 4) {
                faceData.reset(chunkData.take());
                loadFaceData();
            } else if (chunkHeader.type == 5) {
                boneData.reset(chunkData.take());

                bonesCount = *reinterpret_cast<unsigned int*>(boneData.data());

                bones = reinterpret_cast<Bone*>(boneData.data() + 16);

            } else if (chunkHeader.type == 6) {
                boneAttachmentData.reset(chunkData.take());

                int count = *reinterpret_cast<unsigned int*>(boneAttachmentData.data());
                if (count != vertices) {
                    mError.append(QString("The number of bone attachments differs from the number of vertices in the"
                                          " model."));
                    return false;
                }

                attachments = reinterpret_cast<BoneAttachment*>(boneAttachmentData.data() + 16);

                // Transform all vertices according to the default pose matrix
                for (int j = 0; j < vertices; ++j) {
                    if (attachments[j].count() == 0) {
                        qWarning("WARNING: Vertex without *any* attachments found. SUM(Weights) = 1.0 failed.");
                        continue;
                    }

                    Vector4 result(0, 0, 0, 0);
                    Vector4 resultNormal(0, 0, 0, 0);

                    for (int k = 0; k < attachments[j].count(); ++k) {
                        float weight = attachments[j].weights()[k];
                        const Bone &bone = bones[attachments[j].bones()[k]];

                        Vector4 transformedPos = bone.defaultPoseTransform() * positions[j];
                        transformedPos *= 1 / transformedPos.w();

                        result += weight * transformedPos;
                        resultNormal += weight * (bone.defaultPoseTransform() * normals[j]);
                    }

                    result.data()[2] *= -1;
                    resultNormal.data()[2] *= -1;

                    result.data()[3] = 1;

                    positions[j] = result;
                    normals[j] = resultNormal;
                }

                glBindBufferARB(GL_ARRAY_BUFFER_ARB, positionBuffer);
                glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(Vector4) * vertices, positions, GL_STATIC_DRAW_ARB);

                glBindBufferARB(GL_ARRAY_BUFFER_ARB, normalBuffer);
                glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(Vector4) * vertices, normals, GL_STATIC_DRAW_ARB);

            } else {
                // This should never happen
            }
        }

	return true;
    }

    void Model::close()
    {
	if (positionBuffer)
	{
            glDeleteBuffersARB(1, &positionBuffer);
            positionBuffer = 0;
	}

	if (normalBuffer)
	{
            glDeleteBuffersARB(1, &normalBuffer);
            normalBuffer = 0;
	}

	if (texcoordBuffer)
	{
            glDeleteBuffersARB(1, &texcoordBuffer);
            texcoordBuffer = 0;
	}

        faceGroups.reset();
        faceData.reset();
        vertexData.reset();
        boneData.reset();
        boneAttachmentData.reset();
    }

    struct VertexHeader {
	uint count;
	uint reserved1;
	uint reserved2;
	uint reserved3;
    };

    void Model::loadVertexData()
    {
	VertexHeader *vertexHeader;

        vertexHeader = reinterpret_cast<VertexHeader*>(vertexData.data());

	vertices = vertexHeader->count;

        char* vertexDataStart = vertexData.data() + sizeof(VertexHeader);

        positions = reinterpret_cast<Vector4*>(vertexDataStart);
        normals = reinterpret_cast<Vector4*>(vertexDataStart + sizeof(Vector4) * vertices);
        texCoords = reinterpret_cast<float*>(vertexDataStart + sizeof(Vector4) * vertices * 2);

	glGenBuffersARB(1, &positionBuffer);
	glGenBuffersARB(1, &normalBuffer);
	glGenBuffersARB(1, &texcoordBuffer);

        for (int i = 0; i < vertices; ++i) {
            //positions[i].data()[2] *= -1;
        }

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, positionBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(Vector4) * vertices, positions, GL_STATIC_DRAW_ARB);

	for (int i = 0; i < vertices; ++i) {
            //normals[i].data()[2] *= -1;
	}

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, normalBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(Vector4) * vertices, normals, GL_STATIC_DRAW_ARB);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, texcoordBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(float) * 2 * vertices, texCoords, GL_STATIC_DRAW_ARB);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0); // Unbind array buffer
    }

    struct FacesHeader
    {
	uint groups;
	uint reserved1;
	uint reserved2;
	uint reserved3;
    };

    struct FaceGroupHeader
    {
	int materialId;
	uint elementCount;
	uint elementSize;
	uint reserved;
    };

    void Model::loadFaceData()
    {
        const FacesHeader *header = reinterpret_cast<FacesHeader*>(faceData.data());
	
	faces = header->groups;
        faceGroups.reset(new FaceGroup[faces]);

        char *currentDataPointer = faceData.data() + sizeof(FacesHeader);

	for (int i = 0; i < faces; ++i) {
            FaceGroup *faceGroup = faceGroups.data() + i;
            const FaceGroupHeader *groupHeader = reinterpret_cast<FaceGroupHeader*>(currentDataPointer);
            currentDataPointer += sizeof(FaceGroupHeader);

            faceGroup->elementCount = groupHeader->elementCount;
            if (groupHeader->materialId == -1) {
                faceGroup->material = 0;
            } else {
                faceGroup->material = materialState.data() + groupHeader->materialId;
            }

            uint groupSize = groupHeader->elementCount * groupHeader->elementSize;

            glGenBuffersARB(1, &faceGroup->buffer);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, faceGroup->buffer);
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, groupSize, currentDataPointer, GL_STATIC_DRAW_ARB);

            currentDataPointer += groupSize;
	}

	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0); // Unbind array buffer
    }

    void Model::drawNormals() const
    {
        glLineWidth(1.5);
	glEnable(GL_LINE_SMOOTH);
	glColor3f(0, 0, 1);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);

        for (int i = 0; i < vertices; i += 2) {
            Vector4 &vertex = positions[i];
            Vector4 &normal = normals[i];
            glVertex3fv(vertex.data());
            glVertex3fv((vertex + 15 * normal).data());
	}

	glEnd();

        glPointSize(2);
        glBegin(GL_POINTS);
        glColor3f(1, 0, 0);
        for (int i = 0; i < vertices; i += 2) {
            Vector4 &vertex = positions[i];
            glVertex3fv(vertex.data());
        }
        glEnd();

	glEnable(GL_LIGHTING);
    }

    FaceGroup::FaceGroup() : buffer(0), material(0), elementCount(0)
    {
    }

    FaceGroup::~FaceGroup()
    {
	if (buffer) {
            //glDeleteBuffersARB(1, &buffer);
	}
    }

}
