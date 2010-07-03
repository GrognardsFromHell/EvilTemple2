#include "navigationmesh.h"

#include <vector>

#include <QHash>
#include <QElapsedTimer>

#include "util.h"

namespace EvilTemple {

static uint activeNavigationMeshes = 0;

inline static bool westeast_intersect(float z, int left, int right, float xascent, const Vector4 &from, const Vector4 &to, uint minX, uint maxX, Vector4 &intersection)
{
    // Parallel to the axis -> reject
    if (from.z() == to.z())
        return false;

    if (left > maxX)
        return false;

    if (right < minX)
        return false;

    int zdiff = z - from.z();

    float ix = from.x() + xascent * zdiff;

    intersection.setX(ix);
    intersection.setZ(z);

    return ix >= left && ix <= right;
}

inline static bool northsouth_intersect(float x, int top, int bottom, float zascent, const Vector4 &from, const Vector4 &to, uint minZ, uint maxZ, Vector4 &intersection)
{
    // Parallel to the axis -> reject
    if (from.x() == to.x())
        return false;

    if (top > maxZ)
        return false;

    if (bottom < minZ)
        return false;

    float xdiff = x - from.x();
    float iz = from.z() + zascent * xdiff;

    intersection.setX(x);
    intersection.setZ(iz);

    return iz >= top && iz <= bottom;
}

NavigationMesh::NavigationMesh()
{
    activeNavigationMeshes++;
}

NavigationMesh::~NavigationMesh()
{
    activeNavigationMeshes--;
}

inline Vector4 vectorFromPoint(uint x, uint y)
{
    return Vector4(x, 0, y, 1);
}

/**
  Node markings used by AStar
  */
struct AStarNode {
    const NavMeshRect *rect;
    AStarNode *parent;
    const NavMeshPortal *comingFrom;

    uint costFromStart;
    uint costToGoal;
    uint totalCost;

    bool inClosedSet;
    bool inOpenSet;
};

inline uint getDistanceHeuristic(const AStarNode *node, const Vector4 &point) {
    return (node->comingFrom->center - point).length();
}

inline uint getTraversalCost(const NavMeshPortal *from, const NavMeshPortal *to) {
    return (to->center - from->center).length();
}

bool compareAStarNodes(const AStarNode *a, const AStarNode *b)
{
    return a->totalCost < b->totalCost;
}

bool checkLos(const NavMeshRect *losStartRect, const Vector4 &start, const Vector4 &end)
{
    const NavMeshRect *currentRect = losStartRect;

    Vector4 minPos(_mm_min_ps(start, end));
    Vector4 maxPos(_mm_max_ps(start, end));

    float distance = (end - start).lengthSquared();

    Vector4 intersection(0, 0, 0, 1);

    // Precalculate the slope of the los
    Vector4 diff = end - start;
    float xascent = diff.x() / diff.z();
    float zascent = diff.z() / diff.x();

    // Check with each of the current rects portals, whether the line intersects
    forever {
        // If the end point lies in the current rectangle, we succeeded
        if (end.x() >= currentRect->topLeft.x() && end.x() <= currentRect->bottomRight.x()
            && end.z() >= currentRect->topLeft.z() && end.z() <= currentRect->bottomRight.z())
        {
            // TODO: Take dynamic LOS into account?
            return true;
        }

        bool foundPortal = false;

        foreach (const NavMeshPortal *portal, currentRect->portals) {
            // There are only two axes here, so an if-else will suffice
            if (portal->axis == NorthSouth) {
                if (!northsouth_intersect(portal->center.x(), portal->start, portal->end, zascent, start, end, minPos.z(), maxPos.z(), intersection))
                    continue;
            } else {
                if (!westeast_intersect(portal->center.z(), portal->start, portal->end, xascent, start, end, minPos.x(), maxPos.x(), intersection))
                    continue;
            }

            // If taking this portal increases our current distance from the target, skip it
            float portalDistance = (end - intersection).lengthSquared();
            if (portalDistance >= distance)
                continue;
            distance = portalDistance;

            // Take this portal
            if (portal->sideA == currentRect)
                currentRect = portal->sideB;
            else
                currentRect = portal->sideA;
            foundPortal = true;
            break;
        }

        if (!foundPortal)
            return false;
    }
}

QVector<Vector4> NavigationMesh::findPath(const Vector4 &start, const Vector4 &end) const
{
    // Find first and last navmesh tiles
    const NavMeshRect *startRect = findRect(start);
    const NavMeshRect *endRect = findRect(end);
    QVector<Vector4> result;

    if (!startRect || !endRect) {
        return result;
    }

    /*
     Special case: In the same rect, use a direct path.
     */
    if (startRect == endRect) {
        QVector<Vector4> result;
        result << start << end;
        return result;
    }

    int touchedNodes = 0;

    QList<AStarNode*> openSet;
    QHash<const NavMeshRect*, AStarNode*> rectState;

    NavMeshPortal fauxStartPortal;
    fauxStartPortal.center = start;
    fauxStartPortal.sideA = NULL;
    fauxStartPortal.sideB = NULL;

    // Add all the portals accessible from the start position with correct cost
    AStarNode *startNode = new AStarNode;
    startNode->rect = startRect;
    startNode->comingFrom = &fauxStartPortal;
    startNode->parent = NULL;
    startNode->inOpenSet = true;
    startNode->inClosedSet = false;
    startNode->costFromStart = 0;
    startNode->costToGoal = getDistanceHeuristic(startNode, end);
    startNode->totalCost = startNode->costToGoal;

    // Map portal to node
    rectState[startRect] = startNode;
    openSet.append(startNode);

    AStarNode *lastNode = NULL;

    while (!openSet.isEmpty()) {
        AStarNode *node = openSet.takeFirst();
        node->inOpenSet = false;

        touchedNodes++;

        if (node->rect == endRect) {
            lastNode = node;
            break; // Reached end successfully.
        }

        uint currentCost = node->costFromStart;

        for (int i = 0; i < node->rect->portals.size(); ++i) {
            const NavMeshPortal *portal = node->rect->portals[i];

            if (portal == node->comingFrom)
                continue;

            const NavMeshRect *neighbourRect = (portal->sideA == node->rect) ? portal->sideB : portal->sideA;

            uint neighbourCost = currentCost + getTraversalCost(node->comingFrom, portal);

            QHash<const NavMeshRect*,AStarNode*>::iterator it = rectState.find(neighbourRect);

            AStarNode *otherNode;

            if (it != rectState.end()) {
                otherNode = it.value();

                if ((otherNode->inClosedSet || otherNode->inOpenSet)
                    && otherNode->costFromStart <= neighbourCost)
                    continue; // Skip, we have already a better path to this node
            } else {
                otherNode = new AStarNode;
                rectState[neighbourRect] = otherNode;
            }

            otherNode->comingFrom = portal;
            otherNode->rect = neighbourRect;
            otherNode->parent = node;
            otherNode->costFromStart = neighbourCost;
            otherNode->costToGoal = getDistanceHeuristic(otherNode, end);
            otherNode->totalCost = otherNode->costFromStart + otherNode->costToGoal;
            otherNode->inOpenSet = true;
            otherNode->inClosedSet = false;

            openSet.append(otherNode);
            qSort(openSet.begin(), openSet.end(), compareAStarNodes);
        }

        node->inClosedSet = true;
    }

    if (lastNode) {
        QVector<const NavMeshRect*> rects;

        result.append(end);
        rects.append(endRect);

        AStarNode *node = lastNode;
        while (node && node->rect != startRect) {
            const NavMeshPortal *portal = node->comingFrom;
            result.prepend(portal->center);
            rects.prepend(node->rect);
            node = node->parent;
        }

        result.prepend(start);
        rects.prepend(startRect);

        // Try compressing the path through LOS checks
        for (int i = 0; i < result.size() - 1; ++i) {
            const Vector4 &losStart = result.at(i);
            const NavMeshRect *losStartRect = rects.at(i);

            for (int j = result.size() - 1; j > i + 1; --j) {
                const Vector4 &losEnd = result.at(j);

                if (checkLos(losStartRect, losStart, losEnd)) {
                    // Remove all nodes between losStart and losEnd from the intermediate path
                    result.remove(i + 1, j - (i + 1));
                    rects.remove(i + 1, j - (i + 1));
                    break;
                }
            }
        }
    }

    qDeleteAll(rectState.values());

    return result;
}

bool NavigationMesh::hasLineOfSight(const Vector4 &from, const Vector4 &to) const
{
    const NavMeshRect *startRect = findRect(from);

    if (!startRect || !findRect(to))
        return false;

    return checkLos(startRect, from, to);
}

const NavMeshRect *NavigationMesh::findRect(const Vector4 &position) const
{
    uint x = position.x();
    uint z = position.z();

    const NavMeshRect * const rects = mRectangles.constData();

    for (int i = 0; i < mRectangles.size(); ++i) {
        const NavMeshRect *rect = rects + i;

        if (x >= rect->topLeft.x() && x <= rect->bottomRight.x()
            && z >= rect->topLeft.z() && z <= rect->bottomRight.z()) {
            return rect;
        }
    }

    return NULL;
}

inline QDataStream &operator >>(QDataStream &stream, NavMeshRect &rect)
{
    stream >> rect.topLeft >> rect.bottomRight >> rect.center;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, NavigationMesh &mesh)
{
    uint count;

    stream >> count;

    mesh.mRectangles.resize(count);

    NavMeshRect * const rects = mesh.mRectangles.data();

    for (int i = 0; i < count; ++i) {
         stream >> rects[i];
    }

    stream >> count;

    mesh.mPortals.resize(count);

    NavMeshPortal * const portals = mesh.mPortals.data();

    for (int i = 0; i < count; ++i) {
        NavMeshPortal *portal = portals + i;

        uint sideAIndex, sideBIndex, axis;

        stream >> portal->center >> sideAIndex >> sideBIndex
                >> axis >> portal->start >> portal->end;

        Q_ASSERT(sideAIndex < mesh.mRectangles.size());
        Q_ASSERT(sideBIndex < mesh.mRectangles.size());

        portal->axis = (PortalAxis)axis;
        portal->sideA = rects + sideAIndex;
        portal->sideB = rects + sideBIndex;

        portal->sideA->portals.append(portal);
        portal->sideB->portals.append(portal);
    }

    return stream;
}

QDataStream &operator >>(QDataStream &stream, TaggedRegion &region)
{
    stream >> region.topLeft >> region.bottomRight >> region.center >> region.tag;

    return stream;
}

uint getActiveNavigationMeshes()
{
    return activeNavigationMeshes;
}

}
