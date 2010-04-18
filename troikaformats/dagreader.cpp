
#include "dagreader.h"
#include "virtualfilesystem.h"
#include "material.h"
#include "model.h"
#include "skeleton.h"

#include <QDataStream>

namespace Troika
{

    class DagReaderData
    {
    public:
        VirtualFileSystem *vfs;
        QString filename;

    };

    DagReader::DagReader(VirtualFileSystem *vfs, const QString &filename) : d_ptr(new DagReaderData)
    {
        d_ptr->vfs = vfs;
        d_ptr->filename = filename;
    }

    DagReader::~DagReader()
    {
    }

    MeshModel *DagReader::get()
    {
        // Read the DAG file
        QByteArray data = d_ptr->vfs->openFile(d_ptr->filename);

        if (data.isNull())
            return NULL;

        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        /*
         ToEE uses a bounding box defined in screen-space. We don't use this
         bounding box and use a normal 3d bounding box instead.
         the bounding box is defined as follows:

         float - negative X of the bounding box center offset
         float - negative Y of the bounding box center offset
         float - negative Z of the bounding box center offset
         float - Radius of the bounding box in screen-space
         */
        stream.skipRawData(4 * sizeof(float));

        qint32 objectCount, dataStart;
        stream >> objectCount >> dataStart; // The format allows for more than one object.
        Q_ASSERT(objectCount == 1); // But we can only allow one object per file        
        // TODO: Can we assert that dataStart == stream.device()->pos() ?

        quint32 vertexCount; // Number of vertices
        quint32 faceCount; // Number of triangles
        quint32 vertexDataStart; // Offset in the file where the vertex data starts
        quint32 faceDataStart; // Offset in the file where the face data starts

        stream >> vertexCount >> faceCount >> vertexDataStart >> faceDataStart;

        // Read the vertices
        stream.device()->seek(vertexDataStart);

        QVector<Vertex> vertices(vertexCount);

        for (quint32 i = 0; i < vertexCount; ++i) {
            Vertex &vertex = vertices[i];
            stream >> vertex.positionX >> vertex.positionZ >> vertex.positionY;
            vertex.positionX = - vertex.positionX;
            vertex.positionZ = vertex.positionZ;
            vertex.attachmentCount = 0;
        }

        // Read the faces
        stream.device()->seek(faceDataStart);

        QVector<Face> faces(faceCount);

        for (quint32 i = 0; i < faceCount; ++i) {
            Face &face = faces[i];
            stream >> face.vertices[0] >> face.vertices[1] >> face.vertices[2];
        }

        // Create a single virtual face group
        QSharedPointer<Material> dagMaterial(new Material(Material::DepthArt, "depthart"));
        QSharedPointer<FaceGroup> faceGroup( new FaceGroup(faces, dagMaterial ) );

        QList< QSharedPointer<FaceGroup> > faceGroups;
        faceGroups.append(faceGroup);

        return new MeshModel(faceGroups, vertices);
    }

}
