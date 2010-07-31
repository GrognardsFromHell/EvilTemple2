
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QDataStream>
#include <QElapsedTimer>
#include <QDir>

#include "zonetemplate.h"
#include "util.h"

#include "navigationmeshbuilder.h"

GAMEMATH_ALIGNEDTYPE_PRE struct GAMEMATH_ALIGNEDTYPE_MID TaggedRegion : public AlignedAllocation {
    Vector4 topLeft;
    Vector4 bottomRight;
    Vector4 center;
    QVariant tag;
} GAMEMATH_ALIGNEDTYPE_POST;

typedef QVector<TaggedRegion> RegionLayer;
typedef QHash<QString, RegionLayer> RegionLayers;

struct NavMeshPortal;

GAMEMATH_ALIGNEDTYPE_PRE struct GAMEMATH_ALIGNEDTYPE_MID NavMeshRect : public AlignedAllocation {
    Vector4 topLeft;
    Vector4 bottomRight;
    Vector4 center;

    // Global coordinates
    uint left;
    uint top;
    uint right;
    uint bottom;

    QVector<const NavMeshPortal*> portals;

    GAMEMATH_INLINE void* operator new(size_t size)
    {
            void *result = ALIGNED_MALLOC(size);
            if (!result)
                    throw std::bad_alloc();
            return result;
    }

    GAMEMATH_INLINE void operator delete(void *ptr)
    {
            ALIGNED_FREE(ptr);
    }

    GAMEMATH_INLINE void* operator new[](size_t size)
    {
            void *result = ALIGNED_MALLOC(size);
            if (!result)
                    throw std::bad_alloc();
            return result;
    }

    GAMEMATH_INLINE void operator delete[](void *ptr)
    {
            ALIGNED_FREE(ptr);
    }
} GAMEMATH_ALIGNEDTYPE_POST;

enum PortalAxis {
    NorthSouth,
    WestEast
};

struct NavMeshPortal : public AlignedAllocation {
    Vector4 center;

    NavMeshRect *sideA;
    NavMeshRect *sideB;

    PortalAxis axis; // The axis on which this portal lies
    uint start, end; // The start and end of the portal on the given axis
};

inline static uint qHash(const QPoint &key) {
    return ((key.x() & 0xFFFF) << 16) | (key.y() & 0xFFFF);
}

static const float PixelPerWorldTile = 28.2842703f;

static const int SectorSideLength = 192;

struct ProcessedSector {
    ProcessedSector();

    ProcessedSector *west; // x
    ProcessedSector *north; // z-
    ProcessedSector *south; // z-
    ProcessedSector *east; // x+

    QPoint origin;

    uchar sound[SectorSideLength][SectorSideLength];
    bool used[SectorSideLength][SectorSideLength];
    bool visited[SectorSideLength][SectorSideLength];
    bool walkable[SectorSideLength][SectorSideLength];
    bool flyable[SectorSideLength][SectorSideLength];
    bool cover[SectorSideLength][SectorSideLength];

    uchar negativeHeight[SectorSideLength][SectorSideLength];

    bool hasNegativeHeight;

    /**
          Indicates whether any tile in this sector is reachable.
          */
    bool anyReachable;

    /**
          Due to the akward tile-defs used by toee, this is necessary.
          There are non-blocking tiles outside of the map, at the borders
          of the sectors. We do a flood-fill to exclude these unnecessary areas.
        */
    bool reachable[SectorSideLength][SectorSideLength];
};

inline ProcessedSector::ProcessedSector()
    : hasNegativeHeight(false), anyReachable(false)
{
    memset(reachable, 0, sizeof(reachable));
    memset(negativeHeight, 0, sizeof(negativeHeight));
}

enum TileFlags
{
    TILE_BLOCKS = 1 << 0,
    TILE_SINKS = 1 << 1,
    TILE_CAN_FLY_OVER = 1 << 2,
    TILE_ICY = 1 << 3,
    TILE_NATURAL = 1 << 4,
    TILE_SOUNDPROOF = 1 << 5,
    TILE_INDOOR = 1 << 6,
    TILE_REFLECTIVE = 1 << 7,
    TILE_BLOCKS_VISION = 1 << 8,
    TILE_BLOCKS_UL = 1 << 9,
    TILE_BLOCKS_UM = 1 << 10,
    TILE_BLOCKS_UR = 1 << 11,
    TILE_BLOCKS_ML = 1 << 12,
    TILE_BLOCKS_MM = 1 << 13,
    TILE_BLOCKS_MR = 1 << 14,
    TILE_BLOCKS_LL = 1 << 15,
    TILE_BLOCKS_LM = 1 << 16,
    TILE_BLOCKS_LR = 1 << 17,
    TILE_FLYOVER_UL = 1 << 18,
    TILE_FLYOVER_UM = 1 << 19,
    TILE_FLYOVER_UR = 1 << 20,
    TILE_FLYOVER_ML = 1 << 21,
    TILE_FLYOVER_MM = 1 << 22,
    TILE_FLYOVER_MR = 1 << 23,
    TILE_FLYOVER_LL = 1 << 24,
    TILE_FLYOVER_LM = 1 << 25,
    TILE_FLYOVER_LR = 1 << 26,
    TILE_FLYOVER_COVER = 1 << 27,
};

enum SharedEdge {
    Shared_None = 0,
    Shared_North,
    Shared_East,
    Shared_South,
    Shared_West
};

/**
      Gets the side on which two axis aligned rectangles touch, assuming they're non-intersecting.
    */
inline static SharedEdge getSharedEdge(uint leftA, uint topA, uint rightA, uint bottomA,
                                       uint leftB, uint topB, uint rightB, uint bottomB)
{
    if (leftA == rightB) {
        if (topB < bottomA && bottomB > topA) {
            return Shared_West;
        } else {
            return Shared_None;
        }
    } else if (leftB == rightA) {
        if (topA < bottomB && bottomA > topB) {
            return Shared_East;
        } else {
            return Shared_None;
        }
    } else if (topB == bottomA) {
        if (leftA < rightB && rightA > leftB) {
            return Shared_South;
        } else {
            return Shared_None;
        }
    } else if (topA == bottomB) {
        if (leftB < rightA && rightB > leftA) {
            return Shared_North;
        } else {
            return Shared_None;
        }
    } else {
        return Shared_None;
    }
}

inline static QPoint transStartPos(const QPoint &vector)
{
    static const float factor = 1 / (PixelPerWorldTile / 3.0f);
    return QPoint(vector.x() * factor, vector.y() * factor);
}

inline static ProcessedSector *findSector(QVector<ProcessedSector> &sectors, const QPoint &pos)
{
    for (int i = 0; i < sectors.size(); ++i) {
        ProcessedSector *sector = sectors.data() + i;

        QPoint d = pos - sector->origin;
        if (d.x() >= 0 && d.x() < SectorSideLength && d.y() >= 0 && d.y() < SectorSideLength)
            return sector;
    }

    return NULL;
}

struct ReachabilityWorkItem {
    ProcessedSector *sector;
    QPoint pos;
};

static void findReachableTiles(const QPoint &startPosition, QVector<ProcessedSector> &sectors)
{
    ProcessedSector *sector = findSector(sectors, startPosition);
    if (!sector)
        return;

    ReachabilityWorkItem wi;
    wi.sector = sector;
    wi.pos = startPosition - sector->origin;

    QList<ReachabilityWorkItem> queue;
    queue << wi;

    while (!queue.isEmpty()) {
        ReachabilityWorkItem item = queue.takeFirst();

        sector = item.sector;
        uint x = item.pos.x();
        uint y = item.pos.y();

        if (!sector->walkable[x][y] || sector->flyable[x][y] || sector->reachable[x][y])
            continue;

        sector->anyReachable = true;
        sector->reachable[x][y] = true;

        ReachabilityWorkItem newWi;

        // Find neighbours + add
        if (x == 0 && sector->west) {
            if (!sector->west->visited[SectorSideLength - 1][y]) {
                sector->west->visited[SectorSideLength - 1][y] = true;
                newWi.sector = sector->west;
                newWi.pos = QPoint(SectorSideLength - 1, y);
                queue.append(newWi);
            }
        } else if (x == (SectorSideLength - 1) && sector->east) {
            if (!sector->east->visited[0][y]) {
                sector->east->visited[0][y] = true;
                newWi.sector = sector->east;
                newWi.pos = QPoint(0, y);
                queue.append(newWi);
            }
        }

        if (x < (SectorSideLength - 1)) {
            if (!sector->visited[x+1][y]) {
                sector->visited[x+1][y] = true;
                newWi.sector = sector;
                newWi.pos = QPoint(x + 1, y);
                queue.append(newWi);
            }
        }
        if (x > 0) {
            if (!sector->visited[x-1][y]) {
                sector->visited[x-1][y] = true;
                newWi.sector = sector;
                newWi.pos = QPoint(x - 1, y);
                queue.append(newWi);
            }
        }

        if (y == 0 && sector->north) {
            if (!sector->north->visited[x][SectorSideLength - 1]) {
                sector->north->visited[x][SectorSideLength - 1] = true;
                newWi.sector = sector->north;
                newWi.pos = QPoint(x, SectorSideLength - 1);
                queue.append(newWi);
            }
        } else if (y == (SectorSideLength - 1) && sector->south) {
            if (!sector->south->visited[x][0]) {
                sector->south->visited[x][0] = true;
                newWi.sector = sector->south;
                newWi.pos = QPoint(x, 0);
                queue.append(newWi);
            }
        }

        if (y < (SectorSideLength - 1)) {
            if (!sector->visited[x][y+1]) {
                sector->visited[x][y+1] = true;
                newWi.sector = sector;
                newWi.pos = QPoint(x, y + 1);
                queue.append(newWi);
            }
        }
        if (y > 0) {
            if (!sector->visited[x][y-1]) {
                sector->visited[x][y-1] = true;
                newWi.sector = sector;
                newWi.pos = QPoint(x, y - 1);
                queue.append(newWi);
            }
        }
    }
}

/*
     All tiles that are not reachable by walking from the starting points are flagged as non-walkable.
     The same restriction doesn't apply to "flyover" tiles, since they don't exhibit the odd behaviour of
     being flagged as flyable far outside the reachable map.
     */
static void makeUnreachableTilesBlocking(QVector<ProcessedSector> &sectors)
{
    for (int i = 0; i < sectors.size(); ++i) {
        ProcessedSector *sector = sectors.data() + i;

        if (!sector->anyReachable) {
            memset(sector->walkable, 0, sizeof(sector->walkable));
        } else {
            for (int x = 0; x < SectorSideLength; ++x) {
                for (int y = 0; y < SectorSideLength; ++y) {
                    sector->walkable[x][y] = sector->reachable[x][y] & sector->walkable[x][y];
                }
            }
        }
    }
}

/**
      Marks a tile in a sector as walkable by analysing a bitfield.
      */
template<const uint Flags>
inline void markWalkable(int x, int y, QImage *debugTexture, ProcessedSector *sector, uint bitfield)
{
    if (bitfield & TILE_BLOCKS || bitfield & Flags) {
#ifndef QT_NO_DEBUG
        debugTexture->setPixel(x, y, 0xFFFF0000);
#endif
        sector->walkable[x][y] = false;
    } else {
        sector->walkable[x][y] = true;
    }
}

/**
      Marks a tile in a sector as flyable by analysing a bitfield.
      */
template<const uint Flags>
inline void markFlyable(int x, int y, QImage *debugTexture, ProcessedSector *sector, uint bitfield)
{
    if (bitfield & Flags) {
#ifndef QT_NO_DEBUG
        debugTexture->setPixel(x, y, 0xFF00FF00);
#endif
        sector->flyable[x][y] = true;
        if (bitfield & TILE_FLYOVER_COVER) {
            sector->cover[x][y] = true;
        }
    } else {
        sector->flyable[x][y] = false;
    }
}

static void mergeRectangles(QList<QRect> &rectangles, const uint maxIterations = 100)
{
    bool madeChanges = true;
    uint iteration = 0;

    // Second-pass tries to unify more sector polygons
    for (iteration = 0; (iteration < maxIterations) && madeChanges; ++iteration) {
        madeChanges = false;

        for (int i = 0; i < rectangles.size(); ++i) {
            for (int j = 0; j < rectangles.size(); ++j) {
                if(i == j)
                    continue;

                const QRect &a = rectangles[i];
                uint leftA = a.x();
                uint topA = a.y();
                uint rightA = leftA + a.width();
                uint bottomA = topA + a.height();

                const QRect &b = rectangles[j];
                uint leftB = b.x();
                uint topB = b.y();
                uint rightB = leftB + b.width();
                uint bottomB = topB + b.height();

                // Only consider merging, if the two polygons share an edge
                SharedEdge edge = getSharedEdge(leftA, topA, rightA, bottomA,
                                                leftB, topB, rightB, bottomB);

                if (edge == Shared_None)
                    continue;

                uint areaA = (rightA - leftA) * (bottomA - topA);
                uint areaB = (rightB - leftB) * (bottomB - topB);
                uint maxArea = qMax(areaA, areaB);

                uint t0, t1;

                uint resultingPrimitives = 1;

                switch (edge) {
                case Shared_West:
                case Shared_East:
                    t0 = qMax<uint>(topA, topB);
                    t1 = qMin<uint>(bottomA, bottomB);
                    if (topA != topB)
                        resultingPrimitives++;
                    if (bottomA != bottomB)
                        resultingPrimitives++;
                    break;
                case Shared_North:
                case Shared_South:
                    t0 = qMax<uint>(leftA, leftB);
                    t1 = qMin<uint>(rightA, rightB);
                    if (leftA != leftB)
                        resultingPrimitives++;
                    if (rightA != rightB)
                        resultingPrimitives++;
                    break;
                default:
                    continue;
                }

                if (resultingPrimitives == 1) {
                    leftA = qMin(leftA, leftB);
                    rightA = qMax(rightA, rightB);
                    topA = qMin(topA, topB);
                    bottomA = qMax(bottomA, bottomB);

                    rectangles[i].setCoords(leftA, topA, rightA - 1, bottomA - 1);
                    if (j < i)
                        i--; // Also adjust i for the removed element if it's after j
                    rectangles.removeAt(j--);

                    madeChanges = true;
                } else if (resultingPrimitives == 2) {
                    if (edge == Shared_North || edge == Shared_South) {
                        if (rightB < rightA || leftB > leftA)
                            continue;
                    } else if (edge == Shared_West || edge == Shared_East) {
                        if (bottomB < bottomA || topB > topA)
                            continue;
                    }

                    switch (edge) {
                    case Shared_West:
                    case Shared_East:
                        if (topB == topA)
                            topB = bottomA;
                        else if (bottomB == bottomA)
                            bottomB = topA;
                        else
                            qFatal("FAIL");
                        break;
                    case Shared_North:
                    case Shared_South:
                        if (leftB == leftA)
                            leftB = rightA;
                        else if (rightB == rightA)
                            rightB = leftA;
                        else
                            qFatal("FAIL");
                        break;
                    case Shared_None:
                        Q_ASSERT(false);
                    }

                    switch (edge) {
                    case Shared_West:
                        leftA = leftB;
                        break;
                    case Shared_East:
                        rightA = rightB;
                        break;
                    case Shared_North:
                        topA = topB;
                        break;
                    case Shared_South:
                        bottomA = bottomB;
                        break;
                    case Shared_None:
                        Q_ASSERT(false);
                    }

                    uint newAreaA = (rightA - leftA) * (bottomA - topA);
                    uint newAreaB = (rightB - leftB) * (bottomB - topB);
                    Q_ASSERT(newAreaA + newAreaB == areaA + areaB);

                    // Heuristic: Only merge, if it improves the area of the greater of the two rectangles
                    if (newAreaA > maxArea || newAreaB > maxArea) {
                        rectangles[i].setCoords(leftA, topA, rightA - 1, bottomA - 1);
                        rectangles[j].setCoords(leftB, topB, rightB - 1, bottomB - 1);
                        madeChanges = true;
                    }
                } else if (resultingPrimitives == 3) {
                    // The edges of the additional rectangle
                    uint leftC = 0, topC = 0, rightC = 0, bottomC = 0;

                    // West/North-edge intersection are treated by the other rectangle
                    if (edge == Shared_East) {
                        if (bottomB < bottomA && topB < topA) {
                            // Define new rectangle
                            leftC = leftB;
                            topC = topB;
                            rightC = rightB;
                            bottomC = topA;

                            // Modify B
                            topB = topA;
                            leftB = leftA;

                            // Modify A
                            topA = bottomB;
                        } else if (bottomB > bottomA && topB > topA) {
                            // Define new rectangle
                            leftC = leftB;
                            rightC = rightB;
                            bottomC = bottomB;
                            topC = bottomA;

                            // Modify B
                            leftB = leftA;
                            bottomB = bottomA;

                            // Modify A
                            bottomA = topB;
                        } else if (bottomB < bottomA && topB > topA) {
                            // Define new rectangle
                            leftC = leftA;
                            rightC = rightA;
                            bottomC = bottomA;
                            topC = bottomB;

                            // Modify B
                            leftB = leftA;

                            // Modify A
                            bottomA = topB;
                        } else if (bottomB > bottomA && topB < topA) {
                            // Define new rectangle
                            leftC = leftB;
                            rightC = rightB;
                            bottomC = bottomB;
                            topC = bottomA;

                            // Modify B
                            bottomB = topA;

                            // Modify A
                            rightA = rightB;
                        }
                    } else if (edge == Shared_South) {
                        bool rightIn = rightB < rightA; // Right edge of B inside of A's slab
                        bool leftIn = leftB > leftA; // Left edge of B inside of A's slab

                        if (leftIn && !rightIn) {
                            leftC = rightA;
                            rightC = rightB;
                            topC = topB;
                            bottomC = bottomB;

                            rightB = rightA;
                            topB = topA;

                            rightA = leftB;
                        } else if (!leftIn && rightIn) {
                            leftC = rightB;
                            rightC = rightA;
                            topC = topA;
                            bottomC = bottomA;

                            rightA = rightB;
                            bottomA = bottomB;

                            rightB = leftA;
                        } else if (leftIn && rightIn) {
                            leftC = rightB;
                            topC = topA;
                            rightC = rightA;
                            bottomC = bottomA;

                            rightA = leftB;
                            topB = topA;
                        } else if (!leftIn && !rightIn) {
                            leftC = rightA;
                            topC = topB;
                            rightC = rightB;
                            bottomC = bottomB;

                            bottomA = bottomB;
                            rightB = leftA;
                        }
                    }

                    uint newAreaA = (rightA - leftA) * (bottomA - topA);
                    uint newAreaB = (rightB - leftB) * (bottomB - topB);
                    uint areaC = (rightC - leftC) * (bottomC - topC);
                    Q_ASSERT(newAreaA + newAreaB + areaC == areaA + areaB);

                    // Heuristic: Only merge, if it improves the area of the greater of the two rectangles
                    if (newAreaA > maxArea || newAreaB > maxArea || areaC > maxArea) {
                        rectangles[i].setCoords(leftA, topA, rightA - 1, bottomA - 1);
                        rectangles[j].setCoords(leftB, topB, rightB - 1, bottomB - 1);

                        QRect newRect;
                        newRect.setCoords(leftC, topC, rightC - 1, bottomC - 1);
                        rectangles.append(newRect);
                        madeChanges = true;
                    }
                }
            }
        }
    }
}

struct WalkablePredicate
{
    inline static bool include(ProcessedSector *sector, int x, int y)
    {
        return sector->walkable[x][y] && !sector->flyable[x][y];
    }
};

struct FlyablePredicate
{
    static bool include(ProcessedSector *sector, int x, int y)
    {
        return sector->walkable[x][y] || sector->flyable[x][y];
    }
};

struct CoverPredicate
{
    static bool include(ProcessedSector *sector, int x, int y)
    {
        return sector->cover[x][y];
    }
};

template<uchar type>
struct FootstepSoundPredicate
{
    static bool include(ProcessedSector *sector, int x, int y)
    {
        return sector->reachable[x][y] && sector->sound[x][y] == type;
    }
};

template<uchar height>
struct HeightPredicate
{
    static bool include(ProcessedSector *sector, int x, int y)
    {
        return sector->negativeHeight[x][y] == height;
    }
};

template<typename InclusionPredicate>
static void findRectangles(QVector<ProcessedSector> &sectors, QList<QRect> &rectangles)
{
    // Reset the used flag for every tile
    for (int i = 0; i < sectors.size(); ++i) {
        ProcessedSector *sector = sectors.data() + i;
        memset(sector->used, 0, sizeof(sector->used));
    }

    for (int si = 0; si < sectors.size(); ++si) {
        ProcessedSector *sector = sectors.data() + si;

        QList<QRect> sectorRectangles;
        QRect rect;

        forever {
            bool foundTile = false;

            // Try to find an unprocessed tile
            for (int x = 0; x < SectorSideLength; ++x) {
                for (int y = 0; y < SectorSideLength; ++y) {
                    if (!sector->used[x][y] && InclusionPredicate::include(sector, x, y)) {
                        rect = QRect(x, y, 1, 1);
                        sector->used[x][y] = true;
                        foundTile = true;
                        break;
                    }
                }
                if (foundTile)
                    break;
            }

            if (!foundTile)
                break;

            forever {
                int right = rect.x() + rect.width();
                int bottom = rect.y() + rect.height();

                // Try expanding the rectangle by one in every direction
                while (right < SectorSideLength) {
                    bool expanded = true;
                    for (int y = rect.top(); y < bottom; ++y) {
                        if (sector->used[right][y] || !InclusionPredicate::include(sector, right, y)) {
                            expanded = false;
                            break;
                        }
                    }

                    if (expanded) {
                        // Mark as used
                        for (int y = rect.top(); y < bottom; ++y) {
                            sector->used[right][y] = true;
                        }
                        rect.setWidth(rect.width() + 1);
                        right++;
                    } else {
                        break;
                    }
                }

                while (bottom < SectorSideLength) {
                    bool expanded = true;
                    for (int x = rect.left(); x < right; ++x) {
                        if (sector->used[x][bottom] || !InclusionPredicate::include(sector, x, bottom)) {
                            expanded = false;
                            break;
                        }
                    }

                    if (expanded) {
                        // Mark as used
                        for (int x = rect.left(); x < right; ++x) {
                            sector->used[x][bottom] = true;
                        }
                        rect.setHeight(rect.height() + 1);
                        bottom++;
                    } else {
                        break;
                    }
                }

                break; // No further expansion was possible
            }

            int sectorX = sector->origin.x();
            int sectorY = sector->origin.y();
            sectorRectangles.append(QRect(sectorX + rect.x(), sectorY + rect.y(), rect.width(), rect.height()));
        }

        // 1st merge on local sectors
        mergeRectangles(sectorRectangles);

        rectangles.append(sectorRectangles);
    }

    // 2nd merge on all rectangles
    mergeRectangles(rectangles, 1);
}

static void findHeightZones(QVector<ProcessedSector> &sectors, QList<QRect> &rectangles)
{
    // Reset the used flag for every tile
    for (int i = 0; i < sectors.size(); ++i) {
        ProcessedSector *sector = sectors.data() + i;
        memset(sector->used, 0, sizeof(sector->used));
    }

    for (int si = 0; si < sectors.size(); ++si) {
        ProcessedSector *sector = sectors.data() + si;

        QList<QRect> sectorRectangles;
        QRect rect;

        forever {
            bool foundTile = false;

            // Try to find an unprocessed tile
            for (int x = 0; x < SectorSideLength; ++x) {
                for (int y = 0; y < SectorSideLength; ++y) {
                    if (!sector->used[x][y] && sector->negativeHeight[x][y] > 0) {
                        rect = QRect(x, y, 1, 1);
                        sector->used[x][y] = true;
                        foundTile = true;
                        break;
                    }
                }
                if (foundTile)
                    break;
            }

            if (!foundTile)
                break;

            forever {
                int right = rect.x() + rect.width();
                int bottom = rect.y() + rect.height();

                // Try expanding the rectangle by one in every direction
                while (right < SectorSideLength) {
                    bool expanded = true;
                    for (int y = rect.top(); y < bottom; ++y) {
                        if (sector->used[right][y] || !sector->negativeHeight[right][y]) {
                            expanded = false;
                            break;
                        }
                    }

                    if (expanded) {
                        // Mark as used
                        for (int y = rect.top(); y < bottom; ++y) {
                            sector->used[right][y] = true;
                        }
                        rect.setWidth(rect.width() + 1);
                        right++;
                    } else {
                        break;
                    }
                }

                while (bottom < SectorSideLength) {
                    bool expanded = true;
                    for (int x = rect.left(); x < right; ++x) {
                        if (sector->used[x][bottom] || !sector->negativeHeight[x][bottom]) {
                            expanded = false;
                            break;
                        }
                    }

                    if (expanded) {
                        // Mark as used
                        for (int x = rect.left(); x < right; ++x) {
                            sector->used[x][bottom] = true;
                        }
                        rect.setHeight(rect.height() + 1);
                        bottom++;
                    } else {
                        break;
                    }
                }

                break; // No further expansion was possible
            }

            int sectorX = sector->origin.x();
            int sectorY = sector->origin.y();
            sectorRectangles.append(QRect(sectorX + rect.x(), sectorY + rect.y(), rect.width(), rect.height()));
        }


        rectangles.append(sectorRectangles);
    }
}

static void findPortals(QList<NavMeshRect*> &rectangles, QList<NavMeshPortal*> &portals)
{
    for (int i = 0; i < rectangles.size(); ++i) {
        NavMeshRect *rect = rectangles.at(i);

        for (int j = 0; j < rectangles.size(); ++j) {
            if (i == j)
                continue;

            NavMeshRect *other = rectangles.at(j);

            SharedEdge edge = getSharedEdge(rect->left, rect->top, rect->right, rect->bottom,
                                            other->left, other->top, other->right, other->bottom);

            if (edge == Shared_None)
                continue;

            bool portalExists = false;

            for (int k = 0; k < other->portals.size(); ++k) {
                const NavMeshPortal *otherPortal = other->portals.at(k);
                if (otherPortal->sideA == rect || otherPortal->sideB == rect) {
                    portalExists = true;
                    break;
                }
            }

            if (portalExists)
                continue;

            NavMeshPortal *portal = new NavMeshPortal;
            portals.append(portal);
            portal->sideA = rect;
            portal->sideB = other;

            switch (edge) {
            case Shared_North:
                portal->start = qMax(rect->left, other->left);
                portal->end = qMin(rect->right, other->right);
                portal->center = Vector4(0.5f * (portal->start + portal->end), 0, rect->top, 1);
                portal->axis = WestEast;
                break;
            case Shared_South:
                portal->start = qMax(rect->left, other->left);
                portal->end = qMin(rect->right, other->right);
                portal->center = Vector4(0.5f * (portal->start + portal->end), 0, rect->bottom, 1);
                portal->axis = WestEast;
                break;
            case Shared_East:
                portal->start = qMax(rect->top, other->top);
                portal->end = qMin(rect->bottom, other->bottom);
                portal->center = Vector4(rect->right, 0, 0.5f * (portal->start + portal->end), 1);
                portal->axis = NorthSouth;
                break;
            case Shared_West:
                portal->start = qMax(rect->top, other->top);
                portal->end = qMin(rect->bottom, other->bottom);
                portal->center = Vector4(rect->left, 0, 0.5f * (portal->start + portal->end), 1);
                portal->axis = NorthSouth;
                break;
            default:
                qFatal("This should not be reached.");
            }

            // Add portal to both.
            rect->portals.append(portal);
            other->portals.append(portal);
        }
    }
}

bool validatePortals(const QList<NavMeshRect*> &rectangles, const QList<NavMeshPortal*> &portals)
{
    bool result = true;

    // Check some invariants for all portals
    foreach (const NavMeshPortal *portal, portals) {
        if (portal->sideA == portal->sideB) {
            qWarning("Looping portal detected.");
            result = false;
        }
        if (portal->axis == NorthSouth) {
            if (portal->center.x() - floor(portal->center.x()) != 0) {
                qWarning("Portal on x north-south axis with non-integral X component.");
                result = false;
            }
        } else if (portal->axis == WestEast) {
            if (portal->center.z() - floor(portal->center.z()) != 0) {
                qWarning("Portal on x north-south axis with non-integral X component.");
                result = false;
            }
        } else {
            qWarning("Portal with invalid axis.");
            result = false;
        }

        const NavMeshRect *sideA = portal->sideA;
        const NavMeshRect *sideB = portal->sideB;

        // Check that the two linked rectangles even share the expected edge
        if (portal->axis == NorthSouth) {
            if (sideA->left == sideB->right || sideA->right == sideB->left) {
                if (sideA->top >= sideB->bottom || sideA->bottom <= sideB->top) {
                    qWarning("Two linked rectangles aren't touching!");
                    result = false;
                }
            } else {
                qWarning("North-South axis portal links two rectangles that don't touch.");
                result = false;
            }
        } else if (portal->axis == WestEast) {
            if (sideA->top == sideB->bottom || sideA->bottom == sideB->top) {
                if (sideA->left >= sideB->right || sideA->right <= sideB->left) {
                    qWarning("Two linked rectangles aren't touching!");
                    result = false;
                }
            } else {
                qWarning("West-East axis portal links two rectangles that don't touch.");
                result = false;
            }
        }

        // Assert that both sideA and sideB are in the list of rectangles
        bool foundA = false, foundB = false;
        foreach (const NavMeshRect *rect, rectangles) {
            if (rect == sideA)
                foundA = true;
            if (rect == sideB)
                foundB = true;
        }

        if (!foundA || !foundB) {
            qWarning("Found portal with disconnected sides.");
            result = false;
        }
    }

    return result;
}

template<typename InclusionPredicate>
static void findNavRectangles(QVector<ProcessedSector> &sectors, QList<NavMeshRect*> &rectangles)
{
    QList<QRect> geometricRectangles;

    findRectangles<InclusionPredicate>(sectors, geometricRectangles);

    foreach (const QRect &tileRect, geometricRectangles) {
        NavMeshRect *rect = new NavMeshRect;
        rect->left = tileRect.left() * (PixelPerWorldTile / 3);
        rect->right = (tileRect.right() + 1) * (PixelPerWorldTile / 3);
        rect->top = tileRect.top() * (PixelPerWorldTile / 3);
        rect->bottom = (tileRect.bottom() + 1) * (PixelPerWorldTile / 3);

        rect->topLeft = Vector4(rect->left, 0, rect->top, 1);
        rect->bottomRight = Vector4(rect->right, 0, rect->bottom, 1);

        rect->center = 0.5f * (rect->topLeft + rect->bottomRight);
        rect->center.setW(1);

        rectangles.append(rect);
    }
}

template<uchar type>
static void buildSoundRegions(QVector<ProcessedSector> &sectors, RegionLayer &layer, const char *tag)
{
    QList<QRect> rectangles;

    findRectangles< FootstepSoundPredicate<type> >(sectors, rectangles);

    foreach (const QRect &rect, rectangles) {
        uint left = rect.left() * (PixelPerWorldTile / 3);
        uint right = (rect.right() + 1) * (PixelPerWorldTile / 3);
        uint top = rect.top() * (PixelPerWorldTile / 3);
        uint bottom = (rect.bottom() + 1) * (PixelPerWorldTile / 3);

        TaggedRegion region;
        region.topLeft = Vector4(left, 0, top, 1);
        region.bottomRight = Vector4(right, 0, bottom, 1);

        region.center = 0.5f * (region.topLeft + region.bottomRight);
        region.center.setW(1);

        region.tag = QVariant(QString::fromLatin1(tag));

        layer.append(region);
    }
}

static void buildSoundLayer(QVector<ProcessedSector> &sectors, RegionLayer &layer)
{
    buildSoundRegions<2>(sectors, layer, "dirt");
    buildSoundRegions<3>(sectors, layer, "grass");
    buildSoundRegions<4>(sectors, layer, "water");
    buildSoundRegions<5>(sectors, layer, "deepWater");
    buildSoundRegions<6>(sectors, layer, "ice");
    buildSoundRegions<7>(sectors, layer, "fire");
    buildSoundRegions<8>(sectors, layer, "wood");
    buildSoundRegions<9>(sectors, layer, "stone");
    buildSoundRegions<10>(sectors, layer, "metal");
    buildSoundRegions<11>(sectors, layer, "marsh");
}

static void buildNegativeHeightRegions(QVector<ProcessedSector> &sectors, RegionLayer &layer)
{
    QList<QRect> rectangles;

    findHeightZones(sectors, rectangles);

    foreach (const QRect &rect, rectangles) {
        uint left = rect.left() * (PixelPerWorldTile / 3);
        uint right = (rect.right() + 1) * (PixelPerWorldTile / 3);
        uint top = rect.top() * (PixelPerWorldTile / 3);
        uint bottom = (rect.bottom() + 1) * (PixelPerWorldTile / 3);

        TaggedRegion region;
        region.topLeft = Vector4(left, 0, top, 1);
        region.bottomRight = Vector4(right, 0, bottom, 1);

        region.center = 0.5f * (region.topLeft + region.bottomRight);
        region.center.setW(1);

        region.tag = QVariant(0x22);

        layer.append(region);
    }
}

static void writeNavigationMesh(QDataStream &stream,
                                const QList<NavMeshRect*> &rectangles,
                                const QList<NavMeshPortal*> &portals)
{
    // Write the navigation mesh out to the file
    stream << (uint)rectangles.size();

    uint index = 0;
    QHash<const NavMeshRect*, uint> rectIndices;

    foreach (const NavMeshRect *rect, rectangles) {
        stream << rect->topLeft << rect->bottomRight << rect->center;
        rectIndices[rect] = index++;
    }

    stream << (uint)portals.size();

    foreach (const NavMeshPortal *portal, portals) {
        Q_ASSERT(rectIndices.contains(portal->sideA));
        Q_ASSERT(rectIndices.contains(portal->sideB));

        stream << portal->center << rectIndices[portal->sideA] << rectIndices[portal->sideB]
                << (uint)portal->axis << portal->start << portal->end;
    }
}

static void writeRegionLayer(QDataStream &stream, const QString &layerName, const RegionLayer &layer)
{
    stream << layerName << layer.size();

    foreach (const TaggedRegion &region, layer) {
        stream << region.topLeft << region.bottomRight << region.center << region.tag;
    }
}

QByteArray NavigationMeshBuilder::build(const Troika::ZoneTemplate *tpl, const QVector<QPoint> &startPositions)
{
    QElapsedTimer timer;
    timer.start();

    QVector<ProcessedSector> sectors;
    sectors.resize(tpl->tileSectors().size());

    qDebug("Generating navigation mesh for %s with %d sectors and %d start locations.",
    qPrintable(tpl->directory()), tpl->tileSectors().size(), startPositions.size());

    for (int i = 0; i < tpl->tileSectors().size(); ++i) {
        const Troika::TileSector &troikaSector = tpl->tileSectors()[i];

        ProcessedSector *sector = sectors.data() + i;
        memset(sector->visited, 0, sizeof(sector->visited));
        memset(sector->used, 0, sizeof(sector->used));

        sector->hasNegativeHeight = troikaSector.hasNegativeHeight;
        for (int x = 0; x < SectorSideLength; ++x)
            for (int y = 0; y < SectorSideLength; ++y)
                sector->negativeHeight[x][y] = troikaSector.negativeHeight[x][y];

        sector->origin = QPoint(troikaSector.x * SectorSideLength, troikaSector.y * SectorSideLength);

        QImage *image = NULL;
#ifndef QT_NO_DEBUG
        image = new QImage(256, 256, QImage::Format_RGB32);
        image->fill(0);
#endif

        uchar footstepSound;
        uint bitfield;

        for (int y = 0; y < 64; ++y) {
            for (int x = 0; x < 64; ++x) {
                bitfield = troikaSector.tiles[x][y].bitfield;
                footstepSound = troikaSector.tiles[x][y].footstepsSound;

                int px = x * 3;
                int py = y * 3;

                // Extract information about walkable tiles from the bitfield
                markWalkable<TILE_BLOCKS_UL>(px    , py    , image, sector, bitfield);
                markWalkable<TILE_BLOCKS_UM>(px + 1, py    , image, sector, bitfield);
                markWalkable<TILE_BLOCKS_UR>(px + 2, py    , image, sector, bitfield);
                markWalkable<TILE_BLOCKS_ML>(px    , py + 1, image, sector, bitfield);
                markWalkable<TILE_BLOCKS_MM>(px + 1, py + 1, image, sector, bitfield);
                markWalkable<TILE_BLOCKS_MR>(px + 2, py + 1, image, sector, bitfield);
                markWalkable<TILE_BLOCKS_LL>(px    , py + 2, image, sector, bitfield);
                markWalkable<TILE_BLOCKS_LM>(px + 1, py + 2, image, sector, bitfield);
                markWalkable<TILE_BLOCKS_LR>(px + 2, py + 2, image, sector, bitfield);

                // Flyover
                markFlyable<TILE_FLYOVER_UL>(px    , py    , image, sector, bitfield);
                markFlyable<TILE_FLYOVER_UM>(px + 1, py    , image, sector, bitfield);
                markFlyable<TILE_FLYOVER_UR>(px + 2, py    , image, sector, bitfield);
                markFlyable<TILE_FLYOVER_ML>(px    , py + 1, image, sector, bitfield);
                markFlyable<TILE_FLYOVER_MM>(px + 1, py + 1, image, sector, bitfield);
                markFlyable<TILE_FLYOVER_MR>(px + 2, py + 1, image, sector, bitfield);
                markFlyable<TILE_FLYOVER_LL>(px    , py + 2, image, sector, bitfield);
                markFlyable<TILE_FLYOVER_LM>(px + 1, py + 2, image, sector, bitfield);
                markFlyable<TILE_FLYOVER_LR>(px + 2, py + 2, image, sector, bitfield);

                for (int tx = px; tx < px + 3; ++tx) {
                    for (int ty = py; ty < py + 3; ++ty) {
                        sector->sound[tx][ty] = footstepSound;
                    }
                }
            }
        }

        // TODO: We could (or should) save the generated texture for further debugging here
    }

    // Link sectors to neighbouring sectors
    for (int i = 0; i < sectors.size(); ++i) {
        ProcessedSector *sector = sectors.data() + i;

        sector->west = 0;
        sector->north = 0;
        sector->south = 0;
        sector->east = 0;

        for (int j = 0; j < sectors.size(); ++j) {
            ProcessedSector *other = sectors.data() + j;

            if (other->origin.x() == sector->origin.x() - SectorSideLength && other->origin.y() == sector->origin.y())
                sector->west = other;
            else if (other->origin.x() == sector->origin.x() + SectorSideLength && other->origin.y() == sector->origin.y())
                sector->east = other;
            else if (other->origin.y() == sector->origin.y() - SectorSideLength && other->origin.x() == sector->origin.x())
                sector->north = other;
            else if (other->origin.y() == sector->origin.y() + SectorSideLength && other->origin.x() == sector->origin.x())
                sector->south = other;
        }
    }

    foreach (const QPoint &startPosition, startPositions) {
        QPoint realStartPos = transStartPos(startPosition);

        findReachableTiles(realStartPos, sectors);
    }

    makeUnreachableTilesBlocking(sectors);

    QList<NavMeshRect*> rectangles;
    findNavRectangles<WalkablePredicate>(sectors, rectangles);

    QList<NavMeshPortal*> portals;
    findPortals(rectangles, portals);

    Q_ASSERT(validatePortals(rectangles, portals));

    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    writeNavigationMesh(stream, rectangles, portals);

    rectangles.clear();
    portals.clear();
    findNavRectangles<FlyablePredicate>(sectors, rectangles);
    findPortals(rectangles, portals);
    Q_ASSERT(validatePortals(rectangles, portals));

    writeNavigationMesh(stream, rectangles, portals);

    RegionLayer waterLayer;
    buildNegativeHeightRegions(sectors, waterLayer);
    writeRegionLayer(stream, "water", waterLayer);

    RegionLayer soundLayer;
    buildSoundLayer(sectors, soundLayer);

    writeRegionLayer(stream, "groundMaterial", soundLayer);

    return result;
}
