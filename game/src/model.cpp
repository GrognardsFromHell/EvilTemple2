
#include <QtOpenGL>
#include "glext.h"

#include "model.h"
#include "material.h"
#include "materials.h"
#include "io/virtualfilesystem.h"

namespace EvilTemple {

    static bool initialized = false;

    PFNGLBINDBUFFERPROC glBindBuffer = NULL;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
    PFNGLGENBUFFERSPROC glGenBuffers = NULL;
    PFNGLBUFFERDATAPROC glBufferData = NULL;
    PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = NULL;
    PFNGLMAPBUFFERPROC glMapBuffer = NULL;
    PFNGLUNMAPBUFFERPROC glUnmapBuffer = NULL;

    static void loadExtensions() {
        const QGLContext *context = QGLContext::currentContext();

        glBindBuffer = (PFNGLBINDBUFFERPROC)context->getProcAddress("glBindBuffer");
        glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)context->getProcAddress("glDeleteBuffers");
        glGenBuffers = (PFNGLGENBUFFERSPROC)context->getProcAddress("glGenBuffers");
        glBufferData = (PFNGLBUFFERDATAPROC)context->getProcAddress("glBufferData");
        glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)context->getProcAddress("glClientActiveTexture");
        glMapBuffer = (PFNGLMAPBUFFERPROC)context->getProcAddress("glMapBuffer");
        glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)context->getProcAddress("glUnmapBuffer");
        initialized = true;
    }

    MeshModel::MeshModel(QList< QSharedPointer<FaceGroup> > faceGroups,
                         const QVector<Vertex> &vertices,
                         Skeleton *skeleton) :
    _faceGroups(faceGroups),
    _vertices(vertices),
    _skeleton(skeleton)
    {
        createBoundingBox();
        createBuffers();
    }

    MeshModel::~MeshModel()
    {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &indexBuffer);
    }

    void MeshModel::createBuffers()
    {
        if (!initialized)
            loadExtensions();

        // Generate the vertex & index buffer id
        glGenBuffers(1, &vertexBuffer);
        glGenBuffers(1, &indexBuffer);

        // A vertex has a position, normal and texture coordinates
        const quint32 vertexSize = sizeof(float) * 3 + sizeof(float) * 3 + sizeof(float) * 2;

        QByteArray bufferData(vertexSize * _vertices.size(), Qt::Uninitialized);

        // Copy all position data into the buffer
        float *floatData = reinterpret_cast<float*>(bufferData.data());
        foreach (const Vertex &vertex, _vertices) {
            *(floatData++) = vertex.positionX;
            *(floatData++) = vertex.positionY;
            *(floatData++) = vertex.positionZ;
        }

        // Followed by all normals
        foreach (const Vertex &vertex, _vertices) {
            *(floatData++) = vertex.normalX;
            *(floatData++) = vertex.normalY;
            *(floatData++) = vertex.normalZ;
        }

        // Followed by all texture coordinates
        foreach (const Vertex &vertex, _vertices) {
            *(floatData++) = vertex.texCoordX;
            *(floatData++) = vertex.texCoordY;
        }

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, bufferData.size(), bufferData.data(), GL_DYNAMIC_DRAW);

        // Create the buffer data for the index buffer
        int faces = 0;
        foreach (const QSharedPointer<FaceGroup> &faceGroup, _faceGroups)
            faces += faceGroup->faces().size();

        int faceSize = 3 * sizeof(quint16);

        bufferData.resize(faceSize * faces);

        quint16 *shortData = reinterpret_cast<quint16*>(bufferData.data());

        foreach (const QSharedPointer<FaceGroup> &faceGroup, _faceGroups) {
            foreach (const Face &face, faceGroup->faces()) {
                for (int i = 0; i < 3; ++i)
                    *(shortData++) = face.vertices[i];
            }
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferData.size(), bufferData.data(), GL_STATIC_DRAW);
    }

    void MeshModel::createBoundingBox()
    {
        if (!_vertices.isEmpty())
        {
            float minX, minY, minZ;
            float maxX, maxY, maxZ;

            const Vertex &firstVertex = _vertices[0];
            minX = maxX = firstVertex.positionX;
            minY = maxY = firstVertex.positionY;
            minZ = maxZ = firstVertex.positionZ;

            for (int i = 1; i < _vertices.count(); ++i)
            {
                const Vertex &vertex = _vertices[i];

                minX = qMin(minX, vertex.positionX);
                maxX = qMax(maxX, vertex.positionX);

                minY = qMin(minY, vertex.positionY);
                maxY = qMax(maxY, vertex.positionY);

                minZ = qMin(minZ, vertex.positionZ);
                maxZ = qMax(maxZ, vertex.positionZ);
            }

            _boundingBox.setExtents(QVector3D(minX, minY, minZ), QVector3D(maxX, maxY, maxZ));
        }
    }

    GLuint MeshModel::createVBO() const
    {
        PFNGLGENBUFFERSPROC glGenBuffersARB =
                (PFNGLGENBUFFERSPROC)QGLContext::currentContext()->getProcAddress("glGenBuffersARB");
        GLuint bufferId;
        glGenBuffersARB(1, &bufferId);
        return bufferId;
    }

    PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;

    void MeshModel::draw(const SkeletonState *skeletonState) const {
        // Use the standard draw method directly.
        if (!skeletonState || !_skeleton) {
            draw();
            return;
        }

        // Both buffers are used by all faces
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

        int vertexCount = _vertices.size();

        float *positionOut = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
        float *normalOut = positionOut + 3 * vertexCount;

        // As above, structure is non-interleaved: position, normals, texcoords
        for (int i = 0; i < vertexCount; ++i) {
            const Vertex &vertex = _vertices[i];

            if (vertex.attachmentCount != 0) {
                QVector3D position = vertex.position();
                QVector4D normal = QVector4D(vertex.normal());

                QVector3D positionResult;
                QVector4D normalResult;

                for (int j = 0; j < vertex.attachmentCount; ++j) {
                    int boneId = vertex.attachmentBone[j];
                    float weight = vertex.attachmentWeight[j];

                    const QMatrix4x4 &fullBoneTransform = skeletonState->getBoneMatrix(boneId);

                    positionResult += weight * (fullBoneTransform * position);
                    normalResult += weight * (fullBoneTransform * normal);
                }

                *(normalOut++) = normalResult.x();
                *(normalOut++) = normalResult.y();
                *(normalOut++) = normalResult.z();
                *(positionOut++) = positionResult.x();
                *(positionOut++) = positionResult.y();
                *(positionOut++) = positionResult.z();
            }
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);

        draw();
    }

    void MeshModel::draw() const {
        QGLContext *context = const_cast<QGLContext*>(QGLContext::currentContext());

        // Both buffers are used by all faces
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

        int faceDrawOffset = 0; // Pointer into the index buffer (byte-based)

        foreach (const QSharedPointer<FaceGroup> &faceGroup, _faceGroups) {
            faceGroup->material()->bind(context);

            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, 0);

            glEnableClientState(GL_NORMAL_ARRAY);
            int firstNormalOffset = sizeof(float) * 3 * _vertices.size();
            glNormalPointer(GL_FLOAT, 0, (GLvoid*)firstNormalOffset);

            for (int i = 0; i < 4; ++i) {
                glClientActiveTexture(GL_TEXTURE0 + i);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                int firstTexCoordOffset = firstNormalOffset * 2; // Normals have the same size as the positions.
                glTexCoordPointer(2, GL_FLOAT, 0, (GLvoid*)firstTexCoordOffset);
            }

            int vertices = 3  * faceGroup->faces().size();

            glDrawElements(GL_TRIANGLES, vertices,  GL_UNSIGNED_SHORT, (GLvoid*)faceDrawOffset);
            faceDrawOffset += vertices * sizeof(quint16);
            
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_INDEX_ARRAY);
            for (int i = 0; i < 4; ++i) {
                glClientActiveTexture(GL_TEXTURE0 + i);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }

            faceGroup->material()->unbind(context);
        }
    }

    void MeshModel::saveAsText(const QString &filename)
    {
        QFile file(filename);

        if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
            return;

        QTextStream stream(&file);

        stream << "Model:" << endl << endl;

        foreach (const Vertex &vertex, _vertices) {
            stream << vertex.positionX << " " << vertex.positionY << " " << vertex.positionZ << endl;
        }

        file.close();
    }

    FaceGroup::FaceGroup(const QVector<Face> &faces, const QSharedPointer<Material> &material) :
            _faces(faces), _material(material) {
    }

    FaceGroup::~FaceGroup() {
    }

    QSharedPointer<Material> FaceGroup::material()
    {        
        return _material;
    }

}
