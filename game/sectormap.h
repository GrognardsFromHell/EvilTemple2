#ifndef SECTORMAP_H
#define SECTORMAP_H

#include <QObject>
#include <QScopedPointer>
#include <QPolygon>

#include "renderable.h"
#include "texture.h"
#include "navigationmesh.h"

#include <QGLBuffer>

namespace EvilTemple {

class Scene;
class SectorMapData;

class Sector : public Renderable
{
Q_OBJECT
public:
    Sector();

    void render(RenderStates &renderStates, MaterialState *overrideMaterial = NULL);

    const Box3d &boundingBox();

    void setNavigationMesh(const SharedNavigationMesh &navigationMesh);

    void setLayer(const RegionLayer &layer);
private:
    void buildBuffers();

    SharedNavigationMesh mNavigationMesh;

    RegionLayer mLayer;

    bool mBuffersInvalid;
    QGLBuffer mVertexBuffer;
    QGLBuffer mColorBuffer;
    QGLBuffer mIndexBuffer;
    QGLBuffer mPortalVertexBuffer;

    Box3d mBoundingBox;
};

inline void Sector::setLayer(const RegionLayer &layer)
{
    mLayer = layer;
}

class SectorMap : public QObject
{
Q_OBJECT
public:
    SectorMap(Scene *scene);
    ~SectorMap();

public slots:
    bool load(const QString &filename) const;

    bool createDebugView() const;

    QVector<Vector4> findPath(const Vector4 &start, const Vector4 &end) const;

    bool hasLineOfSight(const Vector4 &from, const Vector4 &to) const;

    QVariant regionTag(const QString &layer, const Vector4 &at) const;

private:
    QScopedPointer<SectorMapData> d;
};

}

Q_DECLARE_METATYPE(EvilTemple::SectorMap*)

#endif // SECTORMAP_H
