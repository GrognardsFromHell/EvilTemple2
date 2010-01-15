#ifndef GEOMETRYMESHOBJECT_H
#define GEOMETRYMESHOBJECT_H

#include <QSharedPointer>
#include <QGLContext>
#include <QVector3D>

#include "modelsource.h"
#include "qbox3d.h"

namespace EvilTemple
{

    class Game;
    class MeshModel;
    class SkeletonState;

    /**
      This class represents a scene object that displays a geometry mesh object.
      */
    class GeometryMeshObject : public QObject
    {
        Q_OBJECT
    public:
        GeometryMeshObject(QObject *parent = NULL);
        ~GeometryMeshObject();

        /**
          This vector is used as the default scale for new geometry mesh objects.
          */
        static const QVector3D IdentityScale;

        /**
          Gets the model source for this geometry mesh object.
          @return A pointer to the model source for this object. May be null if no model source was specified.
          The returned pointer is owned by this object.
          */
        ModelSource *modelSource() const;

        /**
          Changes the model source.
          @param source The new model source. This object takes ownership of this pointer.
          */
        void setModelSource(ModelSource *source);

        /**
          Gets the three dimension position of this mesh object.
          */
        const QVector3D position() const;

        /**
          Changes the position of this geometry mesh object.
          */
        void setPosition(const QVector3D &position);

        /**
          Gets the scale for this object.
          */
        const QVector3D &scale() const;

        /**
          Changes the scale of this object.
          */
        void setScale(const QVector3D &scale);

        /**
          Gets the rotation that is applied to this object.
          */
        const QQuaternion &rotation() const;

        /**
          Changes the rotation that is being applied to this geometry object.
          */
        void setRotation(const QQuaternion &rotation);

        /**
          Gets the world matrix for this geometry mesh object. If changes to the
          position, scale or rotation occured, the matrix will be automatically updated.
          */
        const QMatrix4x4 &worldMatrix();

        /**
          Gets the transformed AABB of this geometry mesh object.
          */
        const QBox3D &boundingBox();

        /**
          The model currently used by this geometry node. It's possible,
          that this returns NULL if the model is currently not loaded.
          If that is the case, use the model source instead.
          */
        const QSharedPointer<MeshModel> &mesh();

        /**
          Draw this object on the screen. The default graphics state for the pipeline is assumed.

          @param game The root game object.
          @param The currently active GL context.
          */
        void draw(const Game &game, QGLContext *context);

        /**
          Checks whether this mesh intersects the given ray and returns the distance
          from the ray's origin where the closest intersection occured.
          */
        bool intersects(const QLine3D &ray, float &distance);

    protected:

        void invalidateWorldMatrix();
        virtual void updateWorldMatrix();

        QSharedPointer<MeshModel> _model; // Cached instance of the model
        QScopedPointer<ModelSource> _modelSource; // Loader for the model, so it can be unloaded/reloaded

        QVector3D _position; // 3D position of mesh
        QVector3D _scale; // 3D scaling of object (in all directions)
        QQuaternion _rotation; // Rotation of object

        bool _worldMatrixValid; // Flag whether the world matrix is valid
        QMatrix4x4 _worldMatrix; // Cached world matrix

        bool _boundingBoxValid; // Flag whether the bounding box is valid
        QBox3D _boundingBox; // An AABB already transformed by the world matrix

        QScopedPointer<SkeletonState> skeletonState;

        Q_DISABLE_COPY(GeometryMeshObject);
    };

    inline ModelSource *GeometryMeshObject::modelSource() const
    {
        return _modelSource.data();
    }

    inline const QVector3D GeometryMeshObject::position() const
    {
        return _position;
    }

    inline void GeometryMeshObject::setPosition(const QVector3D &position)
    {
        _position = position;
        invalidateWorldMatrix();
    }

    inline const QVector3D &GeometryMeshObject::scale() const
    {
        return _scale;
    }

    inline void GeometryMeshObject::setScale(const QVector3D &scale)
    {
        _scale = scale;
        invalidateWorldMatrix();
    }

    inline const QQuaternion &GeometryMeshObject::rotation() const
    {
        return _rotation;
    }

    inline void GeometryMeshObject::setRotation(const QQuaternion &rotation)
    {
        _rotation = rotation;
        invalidateWorldMatrix();
    }

    inline const QMatrix4x4 &GeometryMeshObject::worldMatrix()
    {
        updateWorldMatrix();
        return _worldMatrix;
    }

    inline void GeometryMeshObject::invalidateWorldMatrix()
    {
        _worldMatrixValid = false;
        _boundingBoxValid = false; // The bounding box depends on the world matrix
    }

    inline const QSharedPointer<MeshModel> &GeometryMeshObject::mesh()
    {
        return _model;
    }

}

#endif // GEOMETRYMESHOBJECT_H
