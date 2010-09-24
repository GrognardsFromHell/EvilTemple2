#include "pathfinder.h"

inline static uint qHash(const QPoint &key)
{
    return ((key.x() << 16) & 0xFFFF0000)
            | (key.y() & 0xFFFF);
}

namespace EvilTemple {

Pathfinder::Pathfinder(QObject *parent) :
    QObject(parent)
{
}

static Vector4 tileToPosition(const QPoint &tile) {
    return Vector4(tile.x() * TileInfo::PixelPerSubtile,
                   0,
                   tile.y() * TileInfo::PixelPerSubtile,
                   1);
}

static QPoint positionToTile(const Vector4 &position) {
    return QPoint(position.x() / TileInfo::PixelPerSubtile, position.z() / TileInfo::PixelPerSubtile);
}

inline static bool canStandAt(const TileInfo *tileInfo, const QPoint &pos, int radius)
{
    int sqradius = radius * radius;

    for (int cx = - radius; cx <= radius; ++cx) {
        int cxs = cx * cx;

        for (int cy = - radius; cy <= radius; ++cy) {
            int dist = cxs + cy * cy;

            if (dist <= sqradius && !tileInfo->isTileWalkable(pos.x() + cx, pos.y() + cy)) {
                return false;
            }
        }
    }

    return true;
}

struct AStarNodeFast {
    QPoint tile;
    const AStarNodeFast *comingFrom;
    uint costFromStart;
    uint costToGoal;
    uint totalCost;

    bool inClosedSet;
    bool inOpenSet;
};

inline static uint getDistanceHeuristic(const QPoint &from, const QPoint &to) {
    return (to - from).manhattanLength();
}

inline static bool compareAStarNodeFasts(const AStarNodeFast *a, const AStarNodeFast *b)
{
    return a->totalCost < b->totalCost;
}

void addNeighbourNodes(const AStarNodeFast *node,
                       const TileInfo *tileInfo,
                       int radius,
                       const QPoint &endTile,
                       QHash<QPoint,AStarNodeFast*> &state,
                       QList<AStarNodeFast*> &openSet);

QVector<Vector4> Pathfinder::findPath(const Vector4 &start, const Vector4 &end, float actorRadius) const
{
    TileInfo *tileInfo = mTileInfo;
    QVector<Vector4> result;

    if (!tileInfo) {
        qWarning("Called Pathfinder::findPath without setting the tile info property first.");
        return result;
    }

    QPoint startTile = positionToTile(start);
    QPoint endTile = positionToTile(end);
    int actorRadiusTiles = (int)ceil(actorRadius / TileInfo::PixelPerSubtile);

    if (!canStandAt(tileInfo, startTile, actorRadiusTiles) || !canStandAt(tileInfo, endTile, actorRadiusTiles))
        return result;

    int touchedNodes = 0;

    QList<AStarNodeFast*> openSet;

    // Add all the portals accessible from the start position with correct cost
    AStarNodeFast *startNode = new AStarNodeFast;
    startNode->comingFrom = NULL;
    startNode->tile = startTile;
    startNode->inOpenSet = true;
    startNode->inClosedSet = false;
    startNode->costFromStart = 0;
    startNode->costToGoal = getDistanceHeuristic(startTile, endTile);
    startNode->totalCost = startNode->costToGoal;
    openSet << startNode;

    AStarNodeFast *lastNode = NULL;

    QHash<QPoint,AStarNodeFast*> state;
    state[startTile] = startNode;

    while (!openSet.isEmpty()) {
        AStarNodeFast *node = openSet.takeFirst();
        node->inOpenSet = false;

        touchedNodes++;

        if (node->tile == endTile) {
            lastNode = node;
            break; // Reached end successfully.
        }

        addNeighbourNodes(node, tileInfo, actorRadiusTiles, endTile, state, openSet);

        node->inClosedSet = true;
    }

    if (lastNode) {
        result.prepend(end);

        const AStarNodeFast *node = lastNode->comingFrom;
        if (node) {
            while (node->comingFrom) {
                result.prepend(tileToPosition(node->tile));
                node = node->comingFrom;
            }
        }

        result.prepend(start);
    }

    qDeleteAll(state.values());

    return result;
}

void addNeighbourNodes(const AStarNodeFast *node,
                       const TileInfo *tileInfo,
                       int radius,
                       const QPoint &endTile,
                       QHash<QPoint, AStarNodeFast*> &state,
                       QList<AStarNodeFast*> &openSet) {
    QPoint neighbourTiles[8] = {
        node->tile + QPoint(-1, -1),
        node->tile + QPoint(1, -1),
        node->tile + QPoint(1, 1),
        node->tile + QPoint(-1, 1),
        node->tile + QPoint(0, -1),
        node->tile + QPoint(0, 1),
        node->tile + QPoint(1, 0),
        node->tile + QPoint(-1, 0)
    };

    for (int i = 0; i < 8; i++) {
        QPoint neighbourTile = neighbourTiles[i];

        uint neighbourCost = node->costFromStart + 1;

        QHash<QPoint, AStarNodeFast*>::iterator it = state.find(neighbourTile);

        AStarNodeFast *otherNode;

        if (it != state.end()) {
            otherNode = it.value();

            if (node->comingFrom == otherNode)
                continue;

            if ((otherNode->inClosedSet || otherNode->inOpenSet)
                && otherNode->costFromStart <= neighbourCost)
                continue; // Skip, we have already a better path to this node
        } else {
            /*
             We defer this check since we'll lookup tiles in the state anyway.
             If the tile can be stood upon once, it'll get added to the state
             and since walkability information is assumed to be invariant during
             a path finding operation, we can assume that finding the state for
             a tile means the tile is walkable.
             */
            if (!canStandAt(tileInfo, neighbourTile, radius))
                continue;

            otherNode = new AStarNodeFast;
            state[neighbourTile] = otherNode;
        }

        otherNode->comingFrom = node;
        otherNode->tile = neighbourTile;
        otherNode->costFromStart = neighbourCost;
        otherNode->costToGoal = getDistanceHeuristic(neighbourTile, endTile);
        otherNode->totalCost = neighbourCost + otherNode->costToGoal;
        otherNode->inOpenSet = true;
        otherNode->inClosedSet = false;

        openSet.append(otherNode);
        qSort(openSet.begin(), openSet.end(), compareAStarNodeFasts);
    }
}

bool Pathfinder::hasLineOfSight(const Vector4 &from, const Vector4 &to) const
{
    TileInfo *tileInfo = mTileInfo;

    if (!tileInfo) {
        qWarning("Called Pathfinder::findPath without setting the tile info property first.");
        return false;
    }

    QPoint startTile = positionToTile(from);
    QPoint endTile = positionToTile(to);

    if (!tileInfo->isTileFlyable(startTile.x(), startTile.y()))
        return false;

    if (!tileInfo->isTileFlyable(endTile.x(), endTile.y()))
        return false;

    int x0 = startTile.x();
    int y0 = startTile.y();
    int x1 = endTile.x();
    int y1 = endTile.y();

    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    float error = 0;
    float deltaerr = 0;
    if (deltax != 0)
        deltaerr = deltay / deltax;
    int ystep;
    int y = y0;
    if (y0 < y1)
        ystep = 1;
    else
        ystep = -1;

    for (int x = x0; x <= x1; ++x) {
        if (steep) {
            if (!tileInfo->isTileFlyable(y, x))
                return false;
        } else {
            if (!tileInfo->isTileFlyable(x, y))
                return false;
        }

        error += deltaerr;
        if (error >= 0.5) {
            y = y + ystep;
            error = error - 1.0;
        }
    }

    return true;
}

}
