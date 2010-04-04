#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QVector2D>

#include "qbox3d.h"

namespace EvilTemple
{

    class CameraData;

    typedef QVector2D WorldPos;

    /**
      The number of pixels per world coordinate unit.
      */
    const float PixelPerWorldTile = 28.2842703f;

    class Camera : public QObject
    {
    Q_OBJECT
    public:
        explicit Camera(QObject *parent = 0);
        ~Camera();

        Q_PROPERTY(QVector2D centeredOn READ centeredOn WRITE centerOn)

        const WorldPos &centeredOn() const;
        void centerOn(const WorldPos &center);

        void setViewport(QRectF viewport);
        QRectF viewport() const;

        float nearPlane() const;
        float farPlane() const;

        /**
          Gets the basic view matrix used to rotate the camera to an isometric view.
          */
        static QMatrix4x4 baseViewMatrix();

        /**
          Calculates the 3D view frustum in world coordinate space.
          */
        QBox3D viewFrustum() const;
    signals:

        void positionChanged();

    public slots:

        void activate(bool billboard = false) const;

        QVector2D worldToView(const WorldPos &worldPos) const;
        QVector3D worldToView(const QVector3D &worldPos) const;
        QVector3D viewToWorld(const QVector3D &screenPos) const;
        QVector3D unproject(const QVector3D &point) const;

        /**
          Moves the camera in screen coordinates.
          */
        void moveView(const QVector2D &diff);

    private:
        QScopedPointer<CameraData> d_ptr;
    };

}

#endif // CAMERA_H
