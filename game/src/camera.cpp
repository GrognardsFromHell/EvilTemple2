
#include <QVector2D>
#include <QtOpenGL>

#include "camera.h"

namespace EvilTemple
{

    class CameraData {
    public:
        CameraData();

        QVector2D centeredOn;
        QMatrix4x4 baseViewMatrix;
        QMatrix4x4 viewMatrix; // World->View coordinates (including translation)
        QMatrix4x4 invertedViewMatrix; // View->World coordinates (including translation)
        QMatrix4x4 billboardViewMatrix; // Translated View->View
        QMatrix4x4 projectionMatrix; // Projection matrix
        QRectF viewport;

        void updateMatrices();
        void updateProjectionMatrix();
        QVector2D worldToView(const QVector2D &worldPos);
        QVector3D worldToView(const QVector3D &worldPos);
        QVector3D viewToWorld(const QVector3D &screenPos);

        static const float nearPlane;
        static const float farPlane;
    };
	
	const float CameraData::nearPlane = 0;
	const float CameraData::farPlane = 3628;

    CameraData::CameraData()
    {
        baseViewMatrix.scale(1, 1, -1);
        baseViewMatrix.rotate(-44.42700648682643, 1, 0, 0); // Taken from ToEE.exe
        baseViewMatrix.rotate(135.0000005619373, 0, 1, 0); // Taken from ToEE.exe
    }
    
    void CameraData::updateMatrices()
    {
        viewMatrix = baseViewMatrix;
        viewMatrix.translate(-centeredOn.x() * PixelPerWorldTile, 0, -centeredOn.y() * PixelPerWorldTile);

        invertedViewMatrix = viewMatrix.inverted(NULL); // Cache the inverse

        billboardViewMatrix.setToIdentity();
        billboardViewMatrix.translate(worldToView(- centeredOn));
    }

    void CameraData::updateProjectionMatrix()
    {
        projectionMatrix.setToIdentity();
        projectionMatrix.translate(0, 0, 0.5); // Screen Z goes from 0 to 1
        projectionMatrix.ortho(- viewport.width() / 2,
                               viewport.width() / 2,
                               - viewport.height() / 2,
                               viewport.height() / 2,
                               nearPlane, farPlane);
    }       

    QVector2D CameraData::worldToView(const QVector2D &worldPos)
    {
        QVector3D adjustedWorldPos(worldPos.x() * PixelPerWorldTile, 0, worldPos.y() * PixelPerWorldTile);

        return QVector2D(baseViewMatrix * adjustedWorldPos);
    }

    QVector3D CameraData::worldToView(const QVector3D &worldPos)
    {
        return baseViewMatrix * worldPos;
    }

    QVector3D CameraData::viewToWorld(const QVector3D &screenPos)
    {
        return baseViewMatrix.inverted() * screenPos;
    }

    Camera::Camera(QObject *parent) :
            QObject(parent),
            d_ptr(new CameraData)
    {
        d_ptr->centeredOn = QVector2D(480, 480);
        d_ptr->updateMatrices();
    }

    Camera::~Camera()
    {
    }

    void Camera::activate(bool billboard) const
    {

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(d_ptr->projectionMatrix.data());

        glMatrixMode(GL_MODELVIEW);
        if (billboard)
        {
            glLoadMatrixd(d_ptr->billboardViewMatrix.data());
        }
        else
        {
            glLoadMatrixd(d_ptr->viewMatrix.data());
        }
    }

    QVector2D Camera::worldToView(const WorldPos &worldPos) const
    {
        return d_ptr->worldToView(worldPos);
    }

    QVector3D Camera::viewToWorld(const QVector3D &screenPos) const
    {
        return d_ptr->viewToWorld(screenPos);
    }

    QVector3D Camera::worldToView(const QVector3D &worldPos) const
    {
        return d_ptr->worldToView(worldPos);
    }

    const WorldPos &Camera::centeredOn() const
    {
        return d_ptr->centeredOn;
    }

    float Camera::nearPlane() const
    {
        return CameraData::nearPlane;
    }

    float Camera::farPlane() const
    {
        return CameraData::farPlane;
    }

    void Camera::setViewport(QRectF viewport)
    {
        d_ptr->viewport = viewport;
        d_ptr->updateProjectionMatrix();
    }

    QRectF Camera::viewport() const
    {
        return d_ptr->billboardViewMatrix.inverted().mapRect(d_ptr->viewport);
    }

    QBox3D Camera::viewFrustum() const
    {
        // Start with the corners of the view frustum in clip space
        QVector3D viewFrustumCorners[8];
        viewFrustumCorners[0] = QVector3D(-1.0f, 1.0f, -1.0f);
        viewFrustumCorners[1] = QVector3D(1.0f, 1.0f, -1.0f);
        viewFrustumCorners[2] = QVector3D(1.0f, -1.0f, -1.0f);
        viewFrustumCorners[3] = QVector3D(-1.0f, -1.0f, -1.0f);
        viewFrustumCorners[4] = QVector3D(-1.0f, 1.0f, 1.0f);
        viewFrustumCorners[5] = QVector3D(1.0f, 1.0f, 1.0f);
        viewFrustumCorners[6] = QVector3D(1.0f, -1.0f, 1.0f);
        viewFrustumCorners[7] = QVector3D(-1.0f, -1.0f, 1.0f);

        // Then transform them into world space
        QMatrix4x4 projectionInverse = d_ptr->projectionMatrix.inverted();
        projectionInverse = d_ptr->invertedViewMatrix * projectionInverse;

        QBox3D aabb;

        for (int i = 0; i < 8; ++i)
        {
            aabb.expand(projectionInverse * viewFrustumCorners[i]);
        }

        return aabb;
    }

    void Camera::centerOn(const WorldPos &center)
    {
        d_ptr->centeredOn = center;
        d_ptr->updateMatrices();

        // Emit camera-position-change signal?
        emit positionChanged();
    }

    void Camera::moveView(const QVector2D &diff)
    {
        // Convert diff from screen coordinates to game coordinates
        QMatrix4x4 matrix = d_ptr->baseViewMatrix.inverted();

        QVector3D wDiff = matrix * QVector3D(- diff.x(), diff.y(), 0);
        centerOn(centeredOn() + QVector2D(wDiff.x() / PixelPerWorldTile, wDiff.z() / PixelPerWorldTile));
    }

    QMatrix4x4 Camera::baseViewMatrix()
    {
        QMatrix4x4 baseViewMatrix;
        baseViewMatrix.scale(1, 1, -1);
        baseViewMatrix.rotate(-44.42700648682643, 1, 0, 0); // Taken from ToEE.exe
        baseViewMatrix.rotate(135.0000005619373, 0, 1, 0); // Taken from ToEE.exe
        return baseViewMatrix;
    }

    QVector3D Camera::unproject(const QVector3D &point) const
    {
        GLint viewport[4] = {0, 0, d_ptr->viewport.width(), d_ptr->viewport.height()};

        GLdouble objX, objY, objZ;

        gluUnProject(point.x(), point.y(), point.z(),
                     d_ptr->viewMatrix.data(), d_ptr->projectionMatrix.data(), viewport,
                     &objX, &objY, &objZ);

        return QVector3D(objX, objY, objZ);
    }

}
