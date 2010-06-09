#include <QSharedDataPointer>
#include <QTime>

#include "skmreader.h"
#include "virtualfilesystem.h"
#include "model.h"
#include "material.h"
#include "troika_materials.h"
#include "skeleton.h"

namespace Troika
{
    struct SkmHeader {
        quint32 boneCount;
        quint32 boneDataOffset;
        quint32 materialCount;
        quint32 materialDataOffset;
        quint32 vertexCount;
        quint32 vertexDataOffset;
        quint32 faceCount;
        quint32 faceDataOffset;
    };

    class SkmReaderData
    {
    public:
        VirtualFileSystem *vfs;
        Materials *materials;
        QString filename;

        SkmHeader header;

        // Data pointers that will be put into the resulting model
        QVector<Bone> bones;
        QVector<Vertex> vertices;
        QList< QSharedPointer<Material> > modelMaterials;
        QList< QList<Face> > faceGroupLists;
        QScopedPointer<Skeleton> skeleton;

        MeshModel *read() {
            QTime timer;
            timer.start();

            QByteArray skmData = vfs->openFile(filename);
			
            if (skmData.isNull()) {
                qWarning("Unknown model file: %s", qPrintable(filename));
                return NULL;
            }

            QDataStream stream(skmData);
            stream.setByteOrder(QDataStream::LittleEndian);
            stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

            readHeader(stream);
            readBones(stream);
            readVertices(stream);
            readMaterials(stream);
            readFaces(stream);

            // Some models (add meshes for instance) have no skeleton and are unanimated
            QByteArray skaData = vfs->openFile(filename.replace(".skm", ".ska", Qt::CaseInsensitive));
            skeleton.reset(new Skeleton(vertices.data(), vertices.size(), bones, skaData, filename));

            QList< QSharedPointer<FaceGroup> > faceGroups;
            for (quint32 i = 0; i < header.materialCount; ++i) {
                QVector<Face> faces = QVector<Face>::fromList(faceGroupLists[i]);
                                
                QSharedPointer<FaceGroup> faceGroup(new FaceGroup(faces, modelMaterials[i]));
                faceGroups.append(faceGroup);
            }

            MeshModel *result(new MeshModel(faceGroups, vertices, skeleton.take()));

            qDebug("Loaded %s in %d ms.", qPrintable(filename), timer.elapsed());

            return result;
        }

        void readHeader(QDataStream &stream) {
            stream >> header.boneCount >> header.boneDataOffset
                    >> header.materialCount >> header.materialDataOffset
                    >> header.vertexCount >> header.vertexDataOffset
                    >> header.faceCount >> header.faceDataOffset;
        }

        void readBones(QDataStream &stream) {
            char rawName[49];
            rawName[48] = 0; // Force null termination

            stream.device()->seek(header.boneDataOffset);
            bones.resize(header.boneCount);

            for (quint32 i = 0; i < header.boneCount; ++i) {

                Bone &bone = bones[i];

                bone.skmOnly = true;
                bone.skaOnly = false;

                bone.id = i;
                stream >> bone.flags >> bone.parentId;
                stream.readRawData(rawName, 48);

                QMatrix4x4 &fullWorldInverse = bone.fullWorldInverse;

                // ToEE stores it matrix col-major, but with only 3 rows (last row is always 0001)
                for (int row = 0; row < 3; ++row) {
                    for (int col = 0; col < 4; ++col) {
                        stream >> fullWorldInverse(row, col);
                    }
                }
                
                bone.name = QString::fromLatin1(rawName);
            }
        }

        void readVertices(QDataStream &stream) {
            stream.device()->seek(header.vertexDataOffset);

            vertices.resize(header.vertexCount);

            for (quint32 i = 0; i < header.vertexCount; ++i) {
                Vertex &vertex = vertices[i];

                stream  >> vertex.positionX >> vertex.positionY >> vertex.positionZ >> vertex.positionW
                        >> vertex.normalX >> vertex.normalY >> vertex.normalZ >> vertex.normalW
                        >> vertex.texCoordX >> vertex.texCoordY
                        >> vertex.unknown
                        >> vertex.attachmentCount;
                for (int i = 0; i < 6; ++i) {
                    stream >> vertex.attachmentBone[i];
                }
                for (int i = 0; i < 6; ++i) {
                    stream >> vertex.attachmentWeight[i];
                }
            }
        }

        void readMaterials(QDataStream &stream)
        {
            char filename[129];
            filename[128] = 0; // Ensure that the string is null-terminated at all times

            stream.device()->seek(header.materialDataOffset);

            // Read the material filenames
            for (quint32 i = 0; i < header.materialCount; ++i) {
                stream.readRawData(filename, 128);

                QString materialName = QString::fromLatin1(filename);

                if (materialName.endsWith(".mdf", Qt::CaseInsensitive)) {
                    modelMaterials.append(materials->loadFromFile(materialName));
                } else {
                    // TODO: Virtual materials that can change during play don't have an associated MDF
                    // E.g: "FACE" and others
                    modelMaterials.append(QSharedPointer<Material>(new Material(Material::Placeholder, materialName)));
                }
            }
        }

        void readFaces(QDataStream &stream)
        {
            struct FaceWithMaterial {
                quint16 materialId;
                Face face;
            } faceWithMaterial;
            Face &face = faceWithMaterial.face;
            quint16 &materialId = faceWithMaterial.materialId;

            // One face group per material
            for (quint32 i = 0; i < header.materialCount; ++i)
                faceGroupLists.append(QList<Face>());

            stream.device()->seek(header.faceDataOffset);

            for (quint32 i = 0; i < header.faceCount; ++i) {

                // The vertices are saved in the wrong order (since they're meant for DirectX)
                stream >> materialId >> face.vertices[0] >> face.vertices[1] >> face.vertices[2];

                faceGroupLists[materialId].append(face);
            }

        }

    };

    SkmReader::SkmReader(VirtualFileSystem *vfs, Materials *materials, const QString &filename) :
            d_ptr(new SkmReaderData)
    {
        d_ptr->vfs = vfs;
        d_ptr->materials = materials;
        d_ptr->filename = filename;
    }

    SkmReader::~SkmReader()
    {
    }

    MeshModel *SkmReader::get()
    {
        return d_ptr->read();
    }

}
