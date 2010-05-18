#include "clippinggeometry.h"

#include <QtOpenGL/QGLBuffer>

#include <gamemath.h>
#include "renderstates.h"

#include "material.h"
#include "materialstate.h"

#include <QtCore/QFile>
#include <QtCore/QDataStream>

#include "util.h"

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

private:
    uint mFaceCount;
    QGLBuffer mPositionBuffer;
    QGLBuffer mIndexBuffer;
};

class ClippingGeometryInstance {
public:
    ClippingGeometryInstance()
    {
    }

    void setWorldMatrix(const Vector4 &position, const Quaternion &rotation, const Vector4 &scale)
    {

        // Old: -44
        Quaternion rot1 = Quaternion::fromAxisAndAngle(1, 0, 0, -0.77539754);
        Matrix4 rotate1matrix = Matrix4::transformation(Vector4(1,1,1,0), rot1, Vector4(0,0,0,0));

        // Old: 90-135
        Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, 2.3561945f);
        Matrix4 rotate2matrix = Matrix4::transformation(Vector4(1,1,1,0), rot2, Vector4(0,0,0,0));

        Matrix4 baseView = rotate1matrix * rotate2matrix;

        Matrix4 baseViewInverse = baseView.inverted();

        mWorldMatrix =
                Matrix4::transformation(Vector4(1,1,1,1), Quaternion(0,0,0,1), position) *

                Matrix4::transformation(Vector4(1,1,1,1), rotation, Vector4(0,0,0,0)) *
                baseViewInverse *
                Matrix4::transformation(Vector4(scale.x(), scale.z(), scale.y(), 0), Quaternion(0,0,0,1), Vector4(0,0,0,0)) *
                baseView;
    }

    void setMesh(const ClippingGeometryMesh *mesh) {
        mMesh = mesh;
    }

    const Matrix4 &worldMatrix() const {
        return mWorldMatrix;
    }

    const ClippingGeometryMesh *mesh() const {
        return mMesh;
    }

private:
    const ClippingGeometryMesh *mMesh;
    Matrix4 mWorldMatrix;
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

    bool load(const QString &filename) {
        QFile clippingFile(filename);

        if (!clippingFile.open(QIODevice::ReadOnly)) {
            return false;
        }

        QDataStream stream(&clippingFile);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        stream.setByteOrder(QDataStream::LittleEndian);

        stream >> mMeshCount >> mInstanceCount;

        mMeshes.reset(new ClippingGeometryMesh[mMeshCount]);
        mInstances.reset(new ClippingGeometryInstance[mInstanceCount]);

        // Read meshes
        for (int i = 0; i < mMeshCount; ++i) {
            uint vertexCount, faceCount;

            stream >> vertexCount >> faceCount;

            QByteArray vertexData = clippingFile.read(vertexCount * sizeof(Vector4));
            QByteArray facesData = clippingFile.read(faceCount * sizeof(quint16));

            if (!mMeshes[i].load(vertexData, facesData, faceCount)) {
                qWarning("Unable to load %d-th mesh in clipping file %s.", i, qPrintable(filename));
                return false;
            }
        }

        // Read instances
        Vector4 position;
        Vector4 scale;
        Quaternion rotation;
        int meshIndex;

        for (int i = 0; i < mInstanceCount; ++i) {
            stream >> position >> rotation >> scale >> meshIndex;

            Q_ASSERT(meshIndex >= 0 && meshIndex < mMeshCount);

            mInstances[i].setMesh(mMeshes.data() + meshIndex);
            mInstances[i].setWorldMatrix(position, rotation, scale);
        }
    }

    void unload() {

    }

    void draw() {

        // Set up material and prepare to only bind new buffers / world matrices as we go along

        for (int i = 0; i < mClippingMaterial.passCount; ++i) {
            MaterialPassState &pass = mClippingMaterial.passes[i];

            pass.program.bind();

                    // Bind texture samplers
            for (int j = 0; j < pass.textureSamplers.size(); ++j) {
            pass.textureSamplers[j].bind();
                    }

            // Set render states
            foreach (const SharedMaterialRenderState &state, pass.renderStates) {
                state->enable();
            }

            for (int j = 0; j < mInstanceCount; ++j) {
                const ClippingGeometryInstance &instance = mInstances[j];
                const ClippingGeometryMesh *mesh = instance.mesh();

                mRenderStates.setWorldMatrix(instance.worldMatrix());

                // Bind uniforms
                for (int j = 0; j < pass.uniforms.size(); ++j) {
                    pass.uniforms[j].bind();
                }

                // Bind attributes
                for (int j = 0; j < pass.attributes.size(); ++j) {
                    MaterialPassAttributeState &attribute = pass.attributes[j];

                    // Bind the correct buffer
                    switch (attribute.bufferType) {
                    case 0:
                        SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->positionBuffer().bufferId()));
                        break;
                    }

                    // Assign the attribute
                    SAFE_GL(glEnableVertexAttribArray(attribute.location));
                    SAFE_GL(glVertexAttribPointer(attribute.location, attribute.binding.components(), attribute.binding.type(),
                                          attribute.binding.normalized(), attribute.binding.stride(), (GLvoid*)attribute.binding.offset()));

                }

                // Draw the actual model
                SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer().bufferId()));
                SAFE_GL(glDrawElements(GL_TRIANGLES, mesh->faceCount(), GL_UNSIGNED_SHORT, 0));

                // Bind attributes
                for (int j = 0; j < pass.attributes.size(); ++j) {
                    MaterialPassAttributeState &attribute = pass.attributes[j];
                    SAFE_GL(glDisableVertexAttribArray(attribute.location));
                }
            }

            // Unbind any previously bound buffers
            SAFE_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

            // Reset render states to default
            foreach (const SharedMaterialRenderState &state, pass.renderStates) {
                state->disable();
            }

            // Unbind textures
            for (int j = 0; j < pass.textureSamplers.size(); ++j) {
                pass.textureSamplers[j].unbind();
            }

            pass.program.unbind();
        }

    }

private:

    RenderStates &mRenderStates;

    MaterialState mClippingMaterial;

    uint mMeshCount;
    uint mInstanceCount;

    QScopedArrayPointer<ClippingGeometryMesh> mMeshes;
    QScopedArrayPointer<ClippingGeometryInstance> mInstances;

};

ClippingGeometry::ClippingGeometry(RenderStates &renderStates) : d(new ClippingGeometryData(renderStates))
{
}

ClippingGeometry::~ClippingGeometry()
{
}

bool ClippingGeometry::load(const QString &filename)
{
    return d->load(filename);
}

void ClippingGeometry::unload()
{
    d->unload();
}

void ClippingGeometry::draw()
{
    d->draw();
}

}
