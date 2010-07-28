
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QCryptographicHash>
#include <QtGui/QImage>

#include "modelfile.h"
#include "util.h"
#include "materials.h"
#include "modelfilereader.h"

namespace EvilTemple {

    static uint activeModels = 0;

    uint getActiveModels()
    {
        return activeModels;
    }

    Model::Model()
        : mAnimations((Animation*)0), positions(0), normals(0), texCoords(0), vertices(0), textureData(0), faces(0),
        mRadius(std::numeric_limits<float>::infinity()), mRadiusSquared(std::numeric_limits<float>::infinity()),
        faceGroups((FaceGroup*)NULL), mNeedsNormalsRecalculated(false),
        mSkeleton(NULL), mBindingPose(NULL)
    {
        activeModels++;
    }

    Model::~Model()
    {
        activeModels--;
        delete mSkeleton;
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
                    SharedTexture texture = GlobalTextureCache::instance()->get(mMd5Hashes[textureId]);

                    if (!texture) {
                        QByteArray textureData = QByteArray::fromRawData((char*)mTextures[textureId],
                                                                         mTextureSizes[textureId]);

                        texture = SharedTexture(new Texture);
                        if (!texture->loadTga(textureData)) {
                            qWarning("Unable to load model texture.");
                        }

                        GlobalTextureCache::instance()->insert(mMd5Hashes[textureId], texture);
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

    bool Model::load(const QString &filename, Materials *materials, const RenderStates &renderState)
    {
        mError.clear();

        ModelFileReader reader;

        if (!reader.open(filename)) {
            mError.append(QString("Unable to open model file %1: %2").arg(filename).arg(reader.error()));
            return false;
        }

        QVector<Md5Hash> hashes;
        QVector<unsigned char*> textures;
        QVector<int> texturesSize;

        while (reader.hasNextChunk()) {
            if (!reader.nextChunk()) {
                mError = QString("Unable to read next chunk from %1: %2").arg(filename).arg(reader.error());
                return false;
            }

            QDataStream &stream = reader.stream();

            if (reader.chunkType() == Chunk_Textures) {
                AlignedPointer chunkData((char*)ALIGNED_MALLOC(reader.chunkSize()));
                stream.readRawData(chunkData.data(), reader.chunkSize());
                textureData.swap(chunkData);

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
            } else if (reader.chunkType() == Chunk_Materials) {
                uint materialCount;

                stream >> materialCount;

                if (materialCount > 0) {
                    mMaterialState.resize(materialCount);

                    ModelTextureSource textureSource(hashes, textures, texturesSize);

                    for (int j = 0; j < materialCount; ++j) {
                        stream.skipRawData(16); // Skip the md5 hash for now

                        QByteArray rawMaterialData;
                        stream >> rawMaterialData;

                        Material material;

                        if (!material.loadFromData(rawMaterialData)) {
                            mError.append(QString("Unable to read material from model %1:\n%2").arg(filename)
                                          .arg(material.error()));
                            return false;
                        }

                        mMaterialState[j].create();

                        if (!mMaterialState[j]->createFrom(material, renderState, &textureSource)) {
                            mError.append(QString("Unable to create material state for model %1:\n%2").arg(filename)
                                          .arg(mMaterialState[j]->error()));
                            return false;
                        }
                    }
                }
            } else if (reader.chunkType() == Chunk_MaterialReferences) {
                Q_ASSERT(mMaterialState.isEmpty()); // Materials and MaterialReferences are exclusive

                QStringList materialNames;
                stream >> materialNames;

                mMaterialState.resize(materialNames.size());

                for (int j = 0; j < materialNames.size(); ++j)
                    mMaterialState[j] = materials->load(materialNames[j]);

            } else if (reader.chunkType() == Chunk_MaterialPlaceholders) {

                stream >> mPlaceholders;

            } else if (reader.chunkType() == Chunk_BoundingVolumes) {
                stream >> mBoundingBox >> mRadius >> mRadiusSquared;
            } else if (reader.chunkType() == Chunk_Geometry) {
                AlignedPointer chunkData((char*)ALIGNED_MALLOC(reader.chunkSize()));
                stream.readRawData(chunkData.data(), reader.chunkSize());
                vertexData.swap(chunkData);
                loadVertexData();
            } else if (reader.chunkType() == Chunk_Faces) {
                AlignedPointer chunkData((char*)ALIGNED_MALLOC(reader.chunkSize()));
                stream.readRawData(chunkData.data(), reader.chunkSize());
                faceData.swap(chunkData);
                loadFaceData();
            } else if (reader.chunkType() == Chunk_Skeleton) {
                delete mSkeleton;
                mSkeleton = new Skeleton;
                mSkeleton->setName(filename);
                stream >> *mSkeleton;
            } else if (reader.chunkType() == Chunk_BindingPose) {

                mBindingPose = new BindingPose;
                stream >> *mBindingPose;

            } else if (reader.chunkType() == Chunk_Animations) {

                uint count;
                stream >> count;
                stream.skipRawData(3 * sizeof(uint)); // Padding

                mAnimations.reset(new Animation[count]);

                for (int j = 0; j < count; ++j) {
                    Animation &animation = mAnimations[j];
                    stream >> animation;
                    mAnimationMap[animation.name()] = &animation;
                }
            } else if (reader.chunkType() == Chunk_AnimationAliases) {
                QHash<QByteArray, QByteArray> animationMap;
                stream >> animationMap;

                foreach (const QByteArray &anim, animationMap.keys()) {
                    QByteArray mapTo = animationMap[anim];
                    if (!mAnimationMap.contains(mapTo)) {
                        qWarning("Animation %s in animation map maps to unknown animation %s.",
                                 anim.constData(), mapTo.constData());
                    } else {
                        mAnimationMap[anim] = mAnimationMap[mapTo];
                    }
                }
            } else {
                qFatal("Unknown chunk type encountered, although asserted before!");
            }
        }

        return true;
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

        for (int i = 0; i < vertices; ++i) {
            positions[i].setZ(positions[i].z() * -1);
            normals[i].setZ(normals[i].z() * -1);
        }

        positionBuffer.upload(positions, sizeof(Vector4) * vertices);
        normalBuffer.upload(normals, sizeof(Vector4) * vertices);
        texcoordBuffer.upload(texCoords, sizeof(float) * 2 * vertices);

        for (int i = 0; i < vertices; ++i) {
            positions[i].setZ(positions[i].z() * -1);
            normals[i].setZ(normals[i].z() * -1);
        }
    }

    struct FacesHeader
    {
        uint groups;
        uint flags;
        uint reserved2;
        uint reserved3;
    };

    enum FacesFlags {
        FacesFlag_RecalculateNormals = 1
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

        if (header->flags & FacesFlag_RecalculateNormals) {
            mNeedsNormalsRecalculated = true;
        } else {
            mNeedsNormalsRecalculated = false;
        }

        faces = header->groups;
        faceGroups.reset(new FaceGroup[faces]);

        char *currentDataPointer = faceData.data() + sizeof(FacesHeader);

        for (int i = 0; i < faces; ++i) {
            FaceGroup *faceGroup = faceGroups.data() + i;
            const FaceGroupHeader *groupHeader = reinterpret_cast<FaceGroupHeader*>(currentDataPointer);
            currentDataPointer += sizeof(FaceGroupHeader);

            if (groupHeader->materialId < 0) {
                faceGroup->material = 0;
                faceGroup->placeholderId = (- groupHeader->materialId) - 1;
            } else {
                faceGroup->material = mMaterialState[groupHeader->materialId].data();
                faceGroup->placeholderId = -1;
            }

            uint groupSize = groupHeader->elementCount * groupHeader->elementSize;

            faceGroup->buffer.upload(currentDataPointer, groupSize);

            // Also keep a copy in system memory for normal recalculation (maybe base this on a flag?)
            faceGroup->indices.resize(groupHeader->elementCount);
            memcpy(faceGroup->indices.data(), currentDataPointer, sizeof(ushort) * groupHeader->elementCount);

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
            const Vector4 &vertex = positions[i];
            const Vector4 &normal = normals[i];
            glVertex3fv(vertex.data());
            glVertex3fv((vertex + 15 * normal).data());
        }

        glEnd();

        glPointSize(2);
        glBegin(GL_POINTS);
        glColor3f(1, 0, 0);
        for (int i = 0; i < vertices; i += 2) {
            const Vector4 &vertex = positions[i];
            glVertex3fv(vertex.data());
        }
        glEnd();

        glEnable(GL_LIGHTING);
    }

    FaceGroup::FaceGroup() : material(0)
    {
    }

    QList<QByteArray> Model::animations() const
    {
        return mAnimationMap.keys();
    }

}
