
#include <QVector3D>
#include <QString>
#include <limits>

#include "model.h"
#include "geometrymeshobject.h"
#include "util.h"
#include "game.h"

namespace EvilTemple {

    const QVector3D GeometryMeshObject::IdentityScale(1, 1, 1);

    GeometryMeshObject::GeometryMeshObject(QObject *parent) :
            QObject(parent),
            _scale(IdentityScale),
            _worldMatrixValid(true),
            _boundingBoxValid(false)
    {
    }

    GeometryMeshObject::~GeometryMeshObject()
    {
    }

    void GeometryMeshObject::setModelSource(ModelSource *source)
    {
        _modelSource.reset(source);
        _model.clear(); // Free currently cached model
        _boundingBoxValid = false; // The bounding box depends on the model
    }

    void GeometryMeshObject::updateWorldMatrix()
    {
        _worldMatrix.setToIdentity();

        if (!_position.isNull())
            _worldMatrix.translate(_position);

        if (!_rotation.isIdentity())
            _worldMatrix.rotate(_rotation);

        if (_scale != IdentityScale)
            _worldMatrix.scale(_scale);

        _worldMatrix.scale(1, 1, -1); // Convert model-space from DirectX to OpenGL

        _worldMatrixValid = true;
    }

    void GeometryMeshObject::draw(const Game &game, QGLContext *context)
    {
		Q_UNUSED(game);
		Q_UNUSED(context);

        // Update the world matrix if neccessary
        if (!_worldMatrixValid)
            updateWorldMatrix();

        // Load the model if neccessary
        // TODO: Use a QFuture here if possible!
        if (!_model) {
            _model = _modelSource->get();

            // Loading the model can fail
            if (!_model)
                return;
        }

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glMultMatrixd(_worldMatrix.data());

        if (!_model->skeleton()) {
            _model->draw();
        } else {            
            if (!skeletonState) {
                skeletonState.reset(new SkeletonState(_model->skeleton()));
                // Copy the default state from every bone of the skeleon
                foreach (const Bone &bone, _model->skeleton()->bones()) {
                    skeletonState->getBoneMatrix(bone.id) = bone.defaultPoseWorld;
                }
            }

            _model->draw(skeletonState.data());
        }

        // _model->drawSkeleton(); // Debugging flag for this

        // DrawDebugCoordinateSystem();

        glPopMatrix();

        // TODO: Introduce a flag for this debugging functionality
        // DrawBox(boundingBox());
    }

    const QBox3D &GeometryMeshObject::boundingBox()
    {
        if (!_worldMatrixValid)
            updateWorldMatrix();

        if (!_model)
        {
            _model = _modelSource->get();

            // Loading the model can fail but should not lead to a crash.
            if (!_model)
            {
                _boundingBox.setNull();
                return _boundingBox;
            }
        }

        if (_boundingBoxValid)
        {
            return _boundingBox;
        }
        else
        {
            _boundingBox = _model->boundingBox().transformed(_worldMatrix);
            _boundingBoxValid = true;
            return _boundingBox;
        }
    }

    bool GeometryMeshObject::intersects(const QLine3D &ray, float &distance)
    {
        if (!_model)
        {
            _model = _modelSource->get();
            if (!_model)
                return false;
        }

        // Transform the ray into object space, so we don't have to        
        // transform every vertex.
        QMatrix4x4 invWorld = worldMatrix().inverted();

        QVector3D objectOrigin = invWorld * ray.origin();
        QVector3D objectDir = QVector3D(invWorld * QVector4D(ray.direction(), 0));

        QLine3D objectRay(objectOrigin, objectDir);

        float foundDistance = std::numeric_limits<float>::infinity();
        bool found = false;

        const QVector<Vertex> &vertices = _model->vertices();
        foreach (const QSharedPointer<FaceGroup> &faceGroup, _model->faceGroups())
        {
            foreach (const Face &face, faceGroup->faces())
            {
                const Vertex &v0 = vertices[face.vertices[0]];
                const Vertex &v1 = vertices[face.vertices[1]];
                const Vertex &v2 = vertices[face.vertices[2]];
                QVector3D p0 = v0.position();
                QVector3D p1 = v1.position();
                QVector3D p2 = v2.position();

                float triangleDistance;
                float u, v;
                if (Intersects(objectRay, p0, p1, p2, triangleDistance, &u, &v))
                {
                    if (triangleDistance < foundDistance)
                    {
                        // Calculate the texture coordinates of the intersected position.
                        // float tu = (1 - u - v) * v0.texCoordX + u * v1.texCoordX + v * v2.texCoordX;
                        // float tv = (1 - u - v) * v0.texCoordY + u * v1.texCoordY + v * v2.texCoordY;

                        // Do a transparency check for the material
                        // if (!faceGroup->material() || faceGroup->material()->hitTest(tu, tv))
                        // {
                            foundDistance = triangleDistance;
                            found = true;
                        // }
                    }
                }
            }
        }

        if (found) {
            distance = foundDistance;
        }

        return found;
    }

}
