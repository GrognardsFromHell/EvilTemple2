#include "navigationmesh.h"

#include <gamemath.h>
using namespace GameMath;

#include <vector>
#include <limits.h>

#include <QRect>
#include <QHash>
#include <QElapsedTimer>

#include "util.h"

namespace EvilTemple {

static uint activeNavigationMeshes = 0;

enum BspNodeType {
    Bsp_HorizontalSplit,
    Bsp_VerticalSplit,
    Bsp_Leaf
};

struct BspNode {

    BspNodeType type;

    union {
        struct {
            // Leaf node
            int startIndex;
            int count;
        };

        struct {
            // Splitter node
            int boundary;
            BspNode *ltNode; // Node for the less-than part
            BspNode *geNode; // Node for the greater-than-or-equal part
        };
    };

};

/**
  Allows faster access to rectangles via a BSP tree.
  */
class RectangleBspTree {
public:
    RectangleBspTree();
    ~RectangleBspTree();

    void build(const QVector<NavMeshRect> &rectangles);

    const NavMeshRect *find(int x, int y) const;

private:
    QVector<BspNode*> mNodes;
    QVector<const NavMeshRect*> mItems;
};

RectangleBspTree::RectangleBspTree()
{
}

RectangleBspTree::~RectangleBspTree()
{
}

const NavMeshRect *RectangleBspTree::find(int x, int y) const
{
    const BspNode *currentNode = mNodes.first();

    int depth = 0;

    forever {
        switch (currentNode->type) {
        case Bsp_HorizontalSplit:
            depth++;
            if (x < currentNode->boundary)
                currentNode = currentNode->ltNode;
            else
                currentNode = currentNode->geNode;
            break;
        case Bsp_VerticalSplit:
            depth++;
            if (y < currentNode->boundary)
                currentNode = currentNode->ltNode;
            else
                currentNode = currentNode->geNode;
            break;
        case Bsp_Leaf:
            for (int i = currentNode->startIndex; i < currentNode->startIndex + currentNode->count; ++i) {
                const NavMeshRect *rect = mItems[i];
                if (rect->left <= x && rect->top <= y
                    && rect->right >= x && rect->bottom >= y)
                    return rect;
            }
            return NULL;
        }
    }
}

/**
  Splits a list of retangles given a horizontal boundary.
  All rectangles that intersect the left half-space are added
  to the lessThan list, all rectangles that intersect the right half-space
  are added to the greaterEqual set. Please note that a rectangle may
  be added to both sets if it intersects the boundary.
  */
static void splitHorizontally(int boundary,
                const QList<const NavMeshRect*> &rectangles,
                QList<const NavMeshRect*> &lessThan,
                QList<const NavMeshRect*> &greaterEqual)
{
    QList<const NavMeshRect*>::const_iterator it;

    for (it = rectangles.begin(); it != rectangles.end(); ++it) {
        const NavMeshRect *rect = *it;

        if (rect->right >= boundary)
            greaterEqual.append(rect);
        if (rect->left < boundary)
            lessThan.append(rect);
    }
}

/**
  This function operates like splitHorizontally, but it interprets the boundary as a positio on the y-axis.
  */
static void splitVertically(int boundary,
                const QList<const NavMeshRect*> &rectangles,
                QList<const NavMeshRect*> &lessThan,
                QList<const NavMeshRect*> &greaterEqual)
{
    QList<const NavMeshRect*>::const_iterator it;

    for (it = rectangles.begin(); it != rectangles.end(); ++it) {
        const NavMeshRect *rect = *it;

        if (rect->bottom >= boundary)
            greaterEqual.append(rect);
        if (rect->top < boundary)
            lessThan.append(rect);
    }
}

struct BspWorkItem {
    int left, right, top, bottom; // Extent of the region to be processed
    QList<const NavMeshRect*> items; // The items for this work item.
    BspNode *node; // The node this work item operates on
    bool horizontal; // Indicates splitting direction
};

static void setBoundingRect(BspWorkItem &item, const QList<const NavMeshRect*> &rectangles)
{
    item.left = INT_MAX;
    item.top = INT_MAX;
    item.right = INT_MIN;
    item.bottom = INT_MIN;

    QList<const NavMeshRect*>::const_iterator it;

    for (it = rectangles.begin(); it != rectangles.end(); ++it) {
        const NavMeshRect *rect = *it;

        if (rect->left < item.left)
            item.left = rect->left;
        if (rect->top < item.top)
            item.top = rect->top;
        if (rect->right > item.right)
            item.right = rect->right;
        if (rect->bottom > item.bottom)
            item.bottom = rect->bottom;
    }

    Q_ASSERT(item.left <= item.right);
    Q_ASSERT(item.top <= item.bottom);
}

/**
  Tries to find a near-optimal split boundary on the x axis for a list of rectangles.
  It attempts to distribute the rectangles evenly among the two resulting sets.
  */
static int findHorizontalBoundary(const BspWorkItem &workItem)
{
    // Create a set of all the X coordinates that may be used as boundaries
    QSet<int> canidates;

    foreach (const NavMeshRect *rect, workItem.items) {
        if (rect->left < workItem.left)
            canidates << workItem.left;
        else
            canidates << rect->left;
    }

    float quality = 1;
    int solution = 0;

    // Attempt to split by each of the canidates and choose the result closest to 0.5f distribution
    foreach (int canidate, canidates) {
        uint lessThan = 0;
        uint greaterThan = 0;

        foreach (const NavMeshRect *rect, workItem.items) {
            if (rect->right >= canidate)
                greaterThan++;
            else
                lessThan++;
        }

        float canidateQuality = fabs(1 - lessThan / (float)greaterThan);
        if (canidateQuality < quality) {
            solution = canidate;
            quality = canidateQuality;
        }
    }

    return solution;
}

/**
  Tries to find a near-optimal split boundary on the y axis for a list of work items.
  */
static int findVerticalBoundary(const BspWorkItem &workItem)
{
    // Create a set of all the X coordinates that may be used as boundaries
    QSet<int> canidates;

    foreach (const NavMeshRect *rect, workItem.items) {
        if (rect->top < workItem.top)
            canidates << workItem.top;
        else
            canidates << rect->top;
    }

    float quality = 1;
    int solution = 0;

    // Attempt to split by each of the canidates and choose the result closest to 0.5f distribution
    foreach (int canidate, canidates) {
        uint lessThan = 0;
        uint greaterThan = 0;

        foreach (const NavMeshRect *rect, workItem.items) {
            if (rect->bottom >= canidate)
                greaterThan++;
            else
                lessThan++;
        }

        float canidateQuality = fabs(1 - lessThan / (float)greaterThan);
        if (canidateQuality < quality) {
            solution = canidate;
            quality = canidateQuality;
        }
    }

    return solution;
}

const uint LeafThreshold = 10; // At most 10 items in a leaf

void RectangleBspTree::build(const QVector<NavMeshRect> &rectangles)
{
    QList<BspWorkItem> workQueue;

    BspWorkItem rootItem;
    for (int i = 0; i < rectangles.size(); ++i)
        rootItem.items.append(rectangles.data() + i);
    setBoundingRect(rootItem, rootItem.items);
    rootItem.node = new BspNode;
    rootItem.horizontal = true;
    mNodes.append(rootItem.node);

    workQueue << rootItem;

    QList<const NavMeshRect*> lessSet, greaterEqualSet;

    while (!workQueue.isEmpty()) {
        BspWorkItem workItem = workQueue.takeFirst();

        // Handle leafs
        if (workItem.items.size() <= LeafThreshold) {
            workItem.node->type = Bsp_Leaf;
            workItem.node->startIndex = mItems.size();
            workItem.node->count = workItem.items.size();
            for (int i = 0; i < workItem.items.size(); ++i)
                mItems.append(workItem.items[i]);
            continue;
        }

        lessSet.clear();
        greaterEqualSet.clear();

        // Handle horizontals
        if (workItem.horizontal) {
            int boundary = findHorizontalBoundary(workItem);
            splitHorizontally(boundary, workItem.items, lessSet, greaterEqualSet);

            // TODO: This could be improved to use a vertical split instead.
            if (lessSet.isEmpty() || greaterEqualSet.isEmpty()) {
                workItem.node->type = Bsp_Leaf;
                workItem.node->startIndex = mItems.size();
                workItem.node->count = workItem.items.size();
                for (int i = 0; i < workItem.items.size(); ++i)
                    mItems.append(workItem.items[i]);
                continue;
            }

            BspWorkItem lesserItem;
            lesserItem.horizontal = false;
            lesserItem.items = lessSet;
            lesserItem.node = new BspNode;
            mNodes.append(lesserItem.node);
            lesserItem.top = workItem.top;
            lesserItem.bottom = workItem.bottom;
            lesserItem.left = workItem.left;
            lesserItem.right = boundary - 1;

            BspWorkItem greaterItem;
            greaterItem.horizontal = false;
            greaterItem.items = greaterEqualSet;
            greaterItem.node = new BspNode;
            mNodes.append(greaterItem.node);
            greaterItem.top = workItem.top;
            greaterItem.bottom = workItem.bottom;
            greaterItem.left = boundary;
            greaterItem.right = workItem.right;

            workQueue << lesserItem << greaterItem;

            workItem.node->type = Bsp_HorizontalSplit;
            workItem.node->ltNode = lesserItem.node;
            workItem.node->geNode = greaterItem.node;
            workItem.node->boundary = boundary;
            continue;
        }

        // Must be a vertical split here
        int boundary = findVerticalBoundary(workItem);
        splitVertically(boundary, workItem.items, lessSet, greaterEqualSet);

        // TODO: This could be improved to use a horizontal split instead.
        if (lessSet.isEmpty() || greaterEqualSet.isEmpty()) {
            workItem.node->type = Bsp_Leaf;
            workItem.node->startIndex = mItems.size();
            workItem.node->count = workItem.items.size();
            for (int i = 0; i < workItem.items.size(); ++i)
                mItems.append(workItem.items[i]);
            continue;
        }

        BspWorkItem lesserItem;
        lesserItem.horizontal = true;
        lesserItem.items = lessSet;
        lesserItem.node = new BspNode;
        mNodes.append(lesserItem.node);
        lesserItem.top = workItem.top;
        lesserItem.bottom = boundary - 1;
        lesserItem.left = workItem.left;
        lesserItem.right = workItem.right;

        BspWorkItem greaterItem;
        greaterItem.horizontal = true;
        greaterItem.items = greaterEqualSet;
        greaterItem.node = new BspNode;
        mNodes.append(greaterItem.node);
        greaterItem.top = boundary;
        greaterItem.bottom = workItem.bottom;
        greaterItem.left = workItem.left;
        greaterItem.right = workItem.right;

        workQueue << lesserItem << greaterItem;

        workItem.node->type = Bsp_VerticalSplit;
        workItem.node->ltNode = lesserItem.node;
        workItem.node->geNode = greaterItem.node;
        workItem.node->boundary = boundary;
    }

}

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

NavigationMesh::NavigationMesh() : mRectangleBspTree(new RectangleBspTree)
{
    activeNavigationMeshes++;
}

NavigationMesh::~NavigationMesh()
{
    delete mRectangleBspTree;
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
        if (end.x() >= currentRect->left && end.x() <= currentRect->right
            && end.z() >= currentRect->top && end.z() <= currentRect->bottom)
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
    /*uint x = position.x();
    uint z = position.z();

    const NavMeshRect * const rects = mRectangles.constData();

    for (int i = 0; i < mRectangles.size(); ++i) {
        const NavMeshRect *rect = rects + i;

        if (x >= rect->left && x <= rect->right
            && z >= rect->top && z <= rect->bottom) {
            return rect;
        }
    }

    return NULL;*/

    return mRectangleBspTree->find(position.x(), position.z());
}

inline QDataStream &operator >>(QDataStream &stream, NavMeshRect &rect)
{
    stream >> rect.left >> rect.top >> rect.right >> rect.bottom >> rect.center;

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

    // Build the BSP index of the rectangles in this navigation mesh
    qDebug("Building BSP tree for %d rectangles.", mesh.mRectangles.size());
    QElapsedTimer timer;
    timer.start();
    mesh.mRectangleBspTree->build(mesh.mRectangles);
    qDebug("Finished in %d milliseconds.", timer.elapsed());

    return stream;
}

QDataStream &operator >>(QDataStream &stream, TaggedRegion &region)
{
    stream >> region.left >> region.top >> region.right >> region.bottom >> region.center >> region.tag;

    return stream;
}

uint getActiveNavigationMeshes()
{
    return activeNavigationMeshes;
}

}
