#ifndef NAVIGATIONMESH_H
#define NAVIGATIONMESH_H

#include <gamemath.h>
using namespace GameMath;

#include <QVector>
#include <QHash>
#include <QString>
#include <QtGlobal>
#include <QSharedPointer>

namespace EvilTemple {

uint getActiveNavigationMeshes();

struct NavMeshPortal;

struct Region {
    Vector4 topLeft;
    Vector4 bottomRight;
    Vector4 center;
};

struct TaggedRegion : public Region {
    QVariant tag;
};

QDataStream &operator >>(QDataStream &stream, TaggedRegion &region);

typedef QVector<TaggedRegion> RegionLayer;
typedef QHash<QString, RegionLayer> RegionLayers;

struct NavMeshRect : public Region {
    QVector<const NavMeshPortal*> portals;
};

enum PortalAxis {
    NorthSouth,
    WestEast
};

struct NavMeshPortal {
    Vector4 center;

    NavMeshRect *sideA;
    NavMeshRect *sideB;

    PortalAxis axis; // The axis on which this portal lies
    uint start, end; // The start and end of the portal on the given axis
};

class NavigationMesh
{
friend QDataStream &operator >>(QDataStream&, NavigationMesh&);
public:
    NavigationMesh();
    NavigationMesh(const QList<NavMeshRect*> &rectangles, const QList<NavMeshPortal*> &portals);
    ~NavigationMesh();

    const QVector<NavMeshRect> &rectangles() const;
    const QVector<NavMeshPortal> &portals() const;

    QVector<Vector4> findPath(const Vector4 &start, const Vector4 &end) const;

    bool hasLineOfSight(const Vector4 &from, const Vector4 &to) const;

    const NavMeshRect *findRect(const Vector4 &position) const;

private:
    QVector<NavMeshRect> mRectangles;
    QVector<NavMeshPortal> mPortals;
};

QDataStream &operator >>(QDataStream &stream, NavigationMesh &mesh);

typedef QSharedPointer<NavigationMesh> SharedNavigationMesh;

inline const QVector<NavMeshRect> &NavigationMesh::rectangles() const
{
    return mRectangles;
}

inline const QVector<NavMeshPortal> &NavigationMesh::portals() const
{
    return mPortals;
}

}

#endif // NAVIGATIONMESH_H
