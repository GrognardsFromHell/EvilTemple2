#include "clippinggeometry.h"

#include <QtOpenGL/QGLBuffer>

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
class ClippingGeometryMesh {
public:
    ClippingGeometryMesh() : mPositionBuffer(QGLBuffer::VertexBuffer), mIndexBuffer(QGLBuffer::IndexBuffer)
    {
    }

    /**
      Constructs the buffers and loads data into them.
      */
    bool load(const QByteArray &vertexData, const QByteArray &faceData, uint faceCount) {
        mFaceCount = faceCount;

        if (!mPositionBuffer.create()) {
            return false;
        }

        if (!mPositionBuffer.bind()) {
            return false;
        }

        mPositionBuffer.allocate(vertexData.data(), vertexData.size());

        if (!mIndexBuffer.create()) {
            return false;
        }

        if (!mIndexBuffer.bind()) {
            return false;
        }

        mIndexBuffer.allocate(faceData.data(), faceData.size());

        return true;
    }

    const QGLBuffer &positionBuffer() const {
        return mPositionBuffer;
    }

    uint faceCount() const {
        return mFaceCount;
    }

    const QGLBuffer &indexBuffer() const {
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
    QGLBuffer mPositionBuffer;
    QGLBuffer mIndexBuffer;
};

class ClippingGeometryInstance : public Renderable, public BufferSource, public DrawStrategy {
public:
    
    void render(RenderStates &renderStates)
    {
        DrawHelper<ClippingGeometryInstance,ClippingGeometryInstance> drawHelper;
        drawHelper.draw(renderStates, mMaterial, *this, *this);
    }

    const Box3d &boundingBox()
    {
        return mMesh->boundingBox();
    }
    
    void setMesh(const ClippingGeometryMesh *mesh) {
        mMesh = mesh;
    }

    const ClippingGeometryMesh *mesh() const {
        return mMesh;
    }

    void setMaterial(MaterialState *material)
    {
        mMaterial = material;
    }

    void draw(const RenderStates &renderStates, MaterialPassState &state) const
    {
        SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mMesh->indexBuffer().bufferId()));
        SAFE_GL(glDrawElements(GL_TRIANGLES, mMesh->faceCount(), GL_UNSIGNED_SHORT, 0));
        SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    GLint buffer(const MaterialPassAttributeState &attribute) const
    {
        switch (attribute.bufferType) {
        case 0:
            return mMesh->positionBuffer().bufferId();
        }

        return -1;
    }

private:
    MaterialState *mMaterial;
    const ClippingGeometryMesh *mMesh;
};

class ClippingGeometryData
{
public:

    ClippingGeometryData(RenderStates &renderStates)
        : mMeshCount(0), mInstanceCount(0), mRenderStates(renderStates) {

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

    ~ClippingGeometryData() {
        unload();
    }

    bool load(const QString &filename, Scene *scene) {
        QFile clippingFile(filename);

        if (!clippingFile.open(QIODevice::ReadOnly)) {
            return false;
        }

        QDataStream stream(&clippingFile);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        stream.setByteOrder(QDataStream::LittleEndian);

        stream >> mMeshCount >> mInstanceCount;

        mMeshes.reset(new ClippingGeometryMesh[mMeshCount]);

        // Read meshes
        for (int i = 0; i < mMeshCount; ++i) {
            uint vertexCount, faceCount;

            Box3d boundingBox;
            float radius, radiusSquared;

            stream >> boundingBox >> radius >> radiusSquared >> vertexCount >> faceCount;

            QByteArray vertexData = clippingFile.read(vertexCount * sizeof(Vector4));
            QByteArray facesData = clippingFile.read(faceCount * sizeof(quint16));

            if (!mMeshes[i].load(vertexData, facesData, faceCount)) {
                qWarning("Unable to load %d-th mesh in clipping file %s.", i, qPrintable(filename));
                return false;
            }

            mMeshes[i].setBoundingBox(boundingBox);
        }

        // Read instances
        Vector4 position;
        Vector4 scale;
        Quaternion rotation;
        int meshIndex;
        
        for (int i = 0; i < mInstanceCount; ++i) {            
            stream >> position >> rotation >> scale >> meshIndex;

            Q_ASSERT(meshIndex >= 0 && meshIndex < mMeshCount);

            QSharedPointer<ClippingGeometryInstance> instance(new ClippingGeometryInstance);
            instance->setMesh(mMeshes.data() + meshIndex);
            instance->setMaterial(&mClippingMaterial);

            SharedSceneNode node(new SceneNode);
            node->setPosition(position);
            node->setScale(scale);
            node->setRotation(rotation);
            node->setRenderCategory(RenderQueue::ClippingGeometry);
            node->attachObject(instance);
            
            scene->addNode(node);
        }

        return true;
    }

    void unload() {
    }

private:

    RenderStates &mRenderStates;

    MaterialState mClippingMaterial;

    uint mMeshCount;
    uint mInstanceCount;

    QScopedArrayPointer<ClippingGeometryMesh> mMeshes;

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
    d->unload();
}

}
