#ifndef NAVIGATIONMESH_H
#define NAVIGATIONMESH_H

#include <gamemath.h>
using namespace GameMath;

#include <QVector>
#include <QtGlobal>
#include <QSharedPointer>

namespace EvilTemple {

struct NavMeshPortal;

class NavMeshRect {
public:
    Vector4 center;
    Vector4 topLeft;
    Vector4 bottomRight;

    // Global coordinates
    uint left;
    uint top;
    uint right;
    uint bottom;

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
public:
    NavigationMesh(const QList<NavMeshRect*> &rectangles, const QList<NavMeshPortal*> &portals);
    ~NavigationMesh();

    const QList<NavMeshRect*> &rectangles() const;
    const QList<NavMeshPortal*> &portals() const;

    QVector<Vector4> findPath(const Vector4 &start, const Vector4 &end) const;

    bool hasLineOfSight(const Vector4 &from, const Vector4 &to) const;

    const NavMeshRect *findRect(const Vector4 &position) const;

private:
    QList<NavMeshRect*> mRectangles;
    QList<NavMeshPortal*> mPortals;
};

typedef QSharedPointer<NavigationMesh> SharedNavigationMesh;

inline const QList<NavMeshRect*> &NavigationMesh::rectangles() const
{
    return mRectangles;
}

inline const QList<NavMeshPortal*> &NavigationMesh::portals() const
{
    return mPortals;
}

}

#endif // NAVIGATIONMESH_H
