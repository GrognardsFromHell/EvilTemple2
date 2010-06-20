#ifndef SECTORMAP_H
#define SECTORMAP_H

#include <QObject>
#include <QScopedPointer>
#include <QPolygon>
#include <QGLBuffer>

#include "renderable.h"
#include "texture.h"
#include "navigationmesh.h"

namespace EvilTemple {

class Scene;
class SectorMapData;

class Sector : public Renderable
{
Q_OBJECT
public:
    Sector();

    void render(RenderStates &renderStates);

    const Box3d &boundingBox();

    void setNavigationMesh(const SharedNavigationMesh &navigationMesh);
private:
    void buildBuffers();

    SharedNavigationMesh mNavigationMesh;

    bool mBuffersInvalid;
    QGLBuffer mVertexBuffer;
    QGLBuffer mColorBuffer;
    QGLBuffer mIndexBuffer;
    QGLBuffer mPortalVertexBuffer;

    Box3d mBoundingBox;
};

class SectorMap : public QObject
{
Q_OBJECT
public:
    SectorMap(Scene *scene);
    ~SectorMap();

public slots:
    bool load(const QVector<Vector4> &startPositions, const QString &filename) const;

    QVector<Vector4> findPath(const Vector4 &start, const Vector4 &end) const;

    bool hasLineOfSight(const Vector4 &from, const Vector4 &to) const;

private:
    QScopedPointer<SectorMapData> d;
};

}

Q_DECLARE_METATYPE(EvilTemple::SectorMap*)

#endif // SECTORMAP_H
