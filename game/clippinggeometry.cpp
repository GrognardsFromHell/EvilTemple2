
#include <GL/glew.h>

#include "clippinggeometry.h"

#include <gamemath.h>
#include "renderstates.h"

#include "material.h"
#include "materialstate.h"

#include <QtCore/QFile>
#include <QtCore/QDataStream>

#include "util.h"
#include "scenenode.h"
#include "renderable.h"
#include "drawhelper.h"
#include "boxrenderable.h"

using namespace GameMath;

namespace EvilTemple {

/**
    A clipping geometry mesh is an untextured mesh that has no normals, and only
    consists of vertex positions and an index list.
  */
class ClippingGeometryMesh : public AlignedAllocation {
public:
    ClippingGeometryMesh()
    {
        glGenBuffers(1, &mPositionBuffer);
        glGenBuffers(1, &mIndexBuffer);
    }

    ~ClippingGeometryMesh()
    {
        glDeleteBuffers(1, &mPositionBuffer);
        glDeleteBuffers(1, &mIndexBuffer);
    }

    /**
      Constructs the buffers and loads data into them.
      */
    bool load(const QByteArray &vertexData, const QByteArray &faceData, uint faceCount) {
        mFaceCount = faceCount;

        glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size(), vertexData.constData(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceData.size(), faceData.constData(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        return true;
    }

    GLuint positionBuffer() const {
        return mPositionBuffer;
    }

    uint faceCount() const {
        return mFaceCount;
    }

    GLuint indexBuffer() const {
        return mIndexBuffer;
    }

    const Box3d &boundingBox() const {
        return mBoundingBox;
    }

    void setBoundingBox(const Box3d &box)
    {
        mBoundingBox = box;
    }

private:
    Box3d mBoundingBox;
    uint mFaceCount;
    GLuint mPositionBuffer;
    GLuint mIndexBuffer;
};

typedef QSharedPointer<ClippingGeometryMesh> SharedClippingGeometryMesh;

class ClippingGeometryInstance : public Renderable, public BufferSource, public DrawStrategy {
public:

    void render(RenderStates &renderStates, MaterialState *overrideMaterial = NULL)
    {
        // glPolygonMode(GL_FRONT, GL_LINE);

        DrawHelper<ClippingGeometryInstance,ClippingGeometryInstance> drawHelper;
        drawHelper.draw(renderStates, overrideMaterial ? overrideMaterial : mMaterial, *this, *this);

        // glPolygonMode(GL_FRONT, GL_FILL);
    }

    const Box3d &boundingBox()
    {
        return mMesh->boundingBox();
    }

    void setMesh(const SharedClippingGeometryMesh &mesh) {
        mMesh = mesh;
    }

    void setMaterial(MaterialState *material)
    {
        mMaterial = material;
    }

    void draw(const RenderStates&, MaterialPassState&) const
    {
        SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mMesh->indexBuffer()));
        SAFE_GL(glDrawElements(GL_TRIANGLES, mMesh->faceCount(), GL_UNSIGNED_SHORT, 0));
    }

    GLint buffer(const MaterialPassAttributeState &attribute) const
    {
        switch (attribute.bufferType) {
        case 0:
            return mMesh->positionBuffer();
        }

        return -1;
    }

private:
    MaterialState *mMaterial;
    SharedClippingGeometryMesh mMesh;
};

class ClippingGeometryData
{
public:

    ClippingGeometryData(RenderStates &renderStates) : mRenderStates(renderStates) {

        QFile clippingMaterialFile(":/material/clipping_material.xml");

        if (!clippingMaterialFile.open(QIODevice::ReadOnly)) {
            qWarning("Unable to open clipping material file.");
            return;
        }

        Material material;

        if (!material.loadFromData(clippingMaterialFile.readAll())) {
            qWarning("Unable to load clipping material file.");
            return;
        }

        // TODO: Exchange this texture source for a default or Empty texture source
        if (!mClippingMaterial.createFrom(material, renderStates, NULL)) {
            qWarning("Unable to create material state for clipping.");
            return;
        }

    }

    bool load(const QString &filename, Scene *scene) {
        QFile clippingFile(filename);

        if (!clippingFile.open(QIODevice::ReadOnly)) {
            return false;
        }

        QDataStream stream(&clippingFile);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        stream.setByteOrder(QDataStream::LittleEndian);

        uint meshCount, instanceCount;

        stream >> meshCount >> instanceCount;

        QList<SharedClippingGeometryMesh> meshes;
        meshes.reserve(meshCount);

        // Read meshes
        for (int i = 0; i < meshCount; ++i) {
            uint vertexCount, faceCount;

            Box3d boundingBox;
            float radius, radiusSquared;

            stream >> boundingBox >> radius >> radiusSquared >> vertexCount >> faceCount;

            QByteArray vertexData = clippingFile.read(vertexCount * sizeof(Vector4));
            QByteArray facesData = clippingFile.read(faceCount * sizeof(quint16));

            SharedClippingGeometryMesh mesh(new ClippingGeometryMesh);

            if (!mesh->load(vertexData, facesData, faceCount)) {
                qWarning("Unable to load %d-th mesh in clipping file %s.", i, qPrintable(filename));
                return false;
            }

            mesh->setBoundingBox(boundingBox);

            meshes.append(mesh);
        }

        // Read instances
        Vector4 position;
        int meshIndex;

        for (int i = 0; i < instanceCount; ++i) {
            stream >> position >> meshIndex;

            Q_ASSERT(meshIndex >= 0 && meshIndex < meshCount);

            ClippingGeometryInstance *instance = new ClippingGeometryInstance;
            instance->setMesh(meshes[meshIndex]);
            instance->setMaterial(&mClippingMaterial);
            instance->setRenderCategory(Renderable::ClippingGeometry);

            SceneNode *node = scene->createNode();
            instance->setParent(node);
            node->setPosition(position);
            node->attachObject(instance);
        }

        return true;
    }

private:
    RenderStates &mRenderStates;

    MaterialState mClippingMaterial;
};

ClippingGeometry::ClippingGeometry(RenderStates &renderStates) : d(new ClippingGeometryData(renderStates))
{
}

ClippingGeometry::~ClippingGeometry()
{
}

bool ClippingGeometry::load(const QString &filename, Scene *scene)
{
    return d->load(filename, scene);
}

void ClippingGeometry::unload()
{
}

}
