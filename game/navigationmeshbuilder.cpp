
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QDataStream>
#include <QElapsedTimer>

#include "texture.h"
#include "scenenode.h"
#include "scene.h"

#include "navigationmesh.h"
#include "navigationmeshbuilder.h"

inline static uint qHash(const QPoint &key) {
    return ((key.x() & 0xFFFF) << 16) | key.y() & 0xFFFF;
}

namespace EvilTemple {

    static const float PixelPerWorldTile = 28.2842703f;

    struct Tile {
        uint flags;
        uchar footsteps;
    };

    struct TileSector {
        TileSector();

        TileSector *west; // x
        TileSector *north; // z-
        TileSector *south; // z-
        TileSector *east; // x+

        QPoint origin;

        bool used[192][192];
        bool visited[192][192];
        bool walkable[192][192];
        bool flyable[192][192];

        bool anyReachable;

        /**
      Due to the akward tile-defs used by toee, this is necessary
      */
        bool reachable[192][192];
    };

    inline TileSector::TileSector()
        : anyReachable(false)
    {
        memset(reachable, 0, sizeof(reachable));
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
        TILE_BLOCKS_UR = 1 << 9,
        TILE_BLOCKS_UM = 1 << 10,
        TILE_BLOCKS_UL = 1 << 11,
        TILE_BLOCKS_MR = 1 << 12,
        TILE_BLOCKS_MM = 1 << 13,
        TILE_BLOCKS_ML = 1 << 14,
        TILE_BLOCKS_LR = 1 << 15,
        TILE_BLOCKS_LM = 1 << 16,
        TILE_BLOCKS_LL = 1 << 17,
        TILE_FLYOVER_UR = 1 << 18,
        TILE_FLYOVER_UM = 1 << 19,
        TILE_FLYOVER_UL = 1 << 20,
        TILE_FLYOVER_MR = 1 << 21,
        TILE_FLYOVER_MM = 1 << 22,
        TILE_FLYOVER_ML = 1 << 23,
        TILE_FLYOVER_LR = 1 << 24,
        TILE_FLYOVER_LM = 1 << 25,
        TILE_FLYOVER_LL = 1 << 26,
        TILE_FLYOVER_COVER = 1 << 27
                         };


    inline static void pushNeighbours(QList<QPoint> &queue, TileSector *sector, QPoint tile)
    {
        uint x = tile.x();
        uint y = tile.y();

        // West
        if (x > 0 && !sector->visited[x - 1][y]) {
            queue.append(QPoint(x - 1, y));
            sector->visited[x - 1][y] = true;
        }

        // North
        if (y > 0 && !sector->visited[x][y - 1]) {
            queue.append(QPoint(x, y - 1));
            sector->visited[x][y - 1] = true;
        }

        // East
        if (x < 191 && !sector->visited[x + 1][y]) {
            queue.append(QPoint(x + 1, y));
            sector->visited[x + 1][y] = true;
        }

        // South
        if (y < 191 && !sector->visited[x][y + 1]) {
            queue.append(QPoint(x, y + 1));
            sector->visited[x][y + 1] = true;
        }
    }

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

    inline static QPoint vectorToPoint(const Vector4 &vector)
    {
        static const float factor = 1 / (PixelPerWorldTile / 3.0f);
        return QPoint(vector.x() * factor, vector.z() * factor);
    }

    inline static TileSector *findSector(QVector<TileSector> &sectors, const QPoint &pos)
    {
        for (int i = 0; i < sectors.size(); ++i) {
            TileSector *sector = sectors.data() + i;

            QPoint d = pos - sector->origin;
            if (d.x() < 192 && d.y() < 192)
                return sector;
        }

        return NULL;
    }

    struct ReachabilityWorkItem {
        TileSector *sector;
        QPoint pos;
    };

    static void findReachableTiles(const Vector4 &startPosition, QVector<TileSector> &sectors)
    {
        // Find the sector corresponding to start Position
        QPoint pos = vectorToPoint(startPosition);

        TileSector *sector = findSector(sectors, pos);
        ReachabilityWorkItem wi;
        wi.sector = sector;
        wi.pos = pos - sector->origin;

        QList<ReachabilityWorkItem> queue;
        queue << wi;

        while (!queue.isEmpty()) {
            ReachabilityWorkItem item = queue.takeFirst();

            sector = item.sector;
            uint x = item.pos.x();
            uint y = item.pos.y();

            if (!sector->walkable[x][y] || sector->flyable[x][y])
                continue;

            sector->anyReachable = true;
            sector->reachable[x][y] = true;

            ReachabilityWorkItem newWi;

            // Find neighbours + add
            if (x == 0 && sector->west) {
                if (!sector->west->visited[191][y]) {
                    sector->west->visited[191][y] = true;
                    newWi.sector = sector->west;
                    newWi.pos = QPoint(191, y);
                    queue.append(newWi);
                }
            } else if (x == 191 && sector->east) {
                if (!sector->east->visited[0][y]) {
                    sector->east->visited[0][y] = true;
                    newWi.sector = sector->east;
                    newWi.pos = QPoint(0, y);
                    queue.append(newWi);
                }
            }

            if (x < 191) {
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
                if (!sector->north->visited[x][191]) {
                    sector->north->visited[x][191] = true;
                    newWi.sector = sector->north;
                    newWi.pos = QPoint(x, 191);
                    queue.append(newWi);
                }
            } else if (y == 191 && sector->south) {
                if (!sector->south->visited[x][0]) {
                    sector->south->visited[x][0] = true;
                    newWi.sector = sector->south;
                    newWi.pos = QPoint(x, 0);
                    queue.append(newWi);
                }
            }

            if (y < 191) {
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

    class NavigationMeshBuilderData
    {
    public:
        QList<NavMeshRect*> rectangles;
        QList<NavMeshPortal*> portals;

        void findRectangles(QVector<TileSector> &sectors);
        void findPortals();
        bool validatePortals();
    };

    NavigationMeshBuilder::NavigationMeshBuilder() : d(new NavigationMeshBuilderData)
    {
    }

    NavigationMeshBuilder::~NavigationMeshBuilder()
    {
    }

    NavigationMesh *NavigationMeshBuilder::build(const QString &filename, const QVector<Vector4> &startPositions)
    {
        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly)) {
            return NULL;
        }

        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        uint count;
        stream >> count;

        if (count == 0) {
            NavigationMesh *result = new NavigationMesh(d->rectangles, d->portals);
            d->rectangles.clear();
            d->portals.clear();
            return result;
        }

        uint secX, secY;

        QVector<TileSector> sectors;
        sectors.resize(count);

        for (uint i = 0; i < count; ++i) {
            TileSector *tileSector = sectors.data() + i;
            memset(tileSector->visited, 0, sizeof(tileSector->visited));
            memset(tileSector->used, 0, sizeof(tileSector->used));

            stream >> secY >> secX;

            tileSector->origin = QPoint(secX * 192, secY * 192);

            QImage image(256, 256, QImage::Format_RGB32);
            image.fill(0);

            uchar footstepSound;
            uint bitfield;

            for (int y = 0; y < 64; ++y) {
                for (int x = 0; x < 64; ++x) {
                    stream >> footstepSound >> bitfield;

                    int px = x * 3;
                    int py = y * 3;

                    // Blocking
                    if (bitfield & TILE_BLOCKS || bitfield & TILE_BLOCKS_UL) {
                        image.setPixel(px + 2, py, 0xFFFF0000);
                        tileSector->walkable[px+2][py] = false;
                    } else {
                        tileSector->walkable[px+2][py] = true;
                    }
                    if (bitfield & TILE_BLOCKS || bitfield & TILE_BLOCKS_UM) {
                        image.setPixel(px + 1, py, 0xFFFF0000);
                        tileSector->walkable[px + 1][py] = false;
                    } else {
                        tileSector->walkable[px + 1][py] = true;
                    }
                    if (bitfield & TILE_BLOCKS || bitfield & TILE_BLOCKS_UR) {
                        image.setPixel(px, py, 0xFFFF0000);
                        tileSector->walkable[px][py] = false;
                    } else {
                        tileSector->walkable[px][py] = true;
                    }

                    if (bitfield & TILE_BLOCKS || bitfield & TILE_BLOCKS_ML) {
                        image.setPixel(px + 2, py + 1, 0xFFFF0000);
                        tileSector->walkable[px + 2][py + 1] = false;
                    } else {
                        tileSector->walkable[px + 2][py + 1] = true;
                    }
                    if (bitfield & TILE_BLOCKS || bitfield & TILE_BLOCKS_MM) {
                        image.setPixel(px + 1, py + 1, 0xFFFF0000);
                        tileSector->walkable[px + 1][py + 1] = false;
                    } else {
                        tileSector->walkable[px + 1][py + 1] = true;
                    }
                    if (bitfield & TILE_BLOCKS || bitfield & TILE_BLOCKS_MR) {
                        image.setPixel(px, py + 1, 0xFFFF0000);
                        tileSector->walkable[px][py + 1] = false;
                    } else {
                        tileSector->walkable[px][py + 1] = true;
                    }

                    if (bitfield & TILE_BLOCKS || bitfield & TILE_BLOCKS_LL) {
                        image.setPixel(px + 2, py + 2, 0xFFFF0000);
                        tileSector->walkable[px + 2][py + 2] = false;
                    } else {
                        tileSector->walkable[px + 2][py + 2] = true;
                    }
                    if (bitfield & TILE_BLOCKS || bitfield & TILE_BLOCKS_LM) {
                        image.setPixel(px + 1, py + 2, 0xFFFF0000);
                        tileSector->walkable[px + 1][py + 2] = false;
                    } else {
                        tileSector->walkable[px + 1][py + 2] = true;
                    }
                    if (bitfield & TILE_BLOCKS || bitfield & TILE_BLOCKS_LR) {
                        image.setPixel(px, py + 2, 0xFFFF0000);
                        tileSector->walkable[px][py + 2] = false;
                    } else {
                        tileSector->walkable[px][py + 2] = true;
                    }

                    // Flyover
                    if (bitfield & TILE_FLYOVER_UL) {
                        image.setPixel(px + 2, py, image.pixel(px + 2, py) | 0xFF00FF00);
                        tileSector->flyable[px + 2][py] = true;
                    } else {
                        tileSector->flyable[px + 2][py] = false;
                    }

                    if (bitfield & TILE_FLYOVER_UM) {
                        image.setPixel(px + 1, py,  image.pixel(px + 1, py) | 0xFF00FF00);
                        tileSector->flyable[px + 1][py] = true;
                    } else {
                        tileSector->flyable[px + 1][py] = false;
                    }
                    if (bitfield & TILE_FLYOVER_UR) {
                        image.setPixel(px, py,  image.pixel(px, py) | 0xFF00FF00);
                        tileSector->flyable[px][py] = true;
                    } else {
                        tileSector->flyable[px][py] = false;
                    }

                    if (bitfield & TILE_FLYOVER_ML) {
                        image.setPixel(px + 2, py + 1,  image.pixel(px + 2, py + 1) | 0xFF00FF00);
                        tileSector->flyable[px + 2][py + 1] = true;
                    } else {
                        tileSector->flyable[px + 2][py + 1] = false;
                    }

                    if (bitfield & TILE_FLYOVER_MM) {
                        image.setPixel(px + 1, py + 1,  image.pixel(px + 1, py + 1) | 0xFF00FF00);
                        tileSector->flyable[px + 1][py + 1] = true;
                    } else {
                        tileSector->flyable[px + 1][py + 1] = false;
                    }
                    if (bitfield & TILE_FLYOVER_MR) {
                        image.setPixel(px, py + 1,  image.pixel(px, py + 1) | 0xFF00FF00);
                        tileSector->flyable[px][py + 1] = true;
                    } else {
                        tileSector->flyable[px][py + 1] = false;
                    }

                    if (bitfield & TILE_FLYOVER_LL) {
                        image.setPixel(px + 2, py + 2,  image.pixel(px + 2, py + 2) | 0xFF00FF00);
                        tileSector->flyable[px + 2][py + 2] = true;
                    } else {
                        tileSector->flyable[px + 2][py + 2] = false;
                    }
                    if (bitfield & TILE_FLYOVER_LM) {
                        image.setPixel(px + 1, py + 2,  image.pixel(px + 1, py + 2) | 0xFF00FF00);
                        tileSector->flyable[px + 1][py + 2] = true;
                    } else {
                        tileSector->flyable[px + 1][py + 2] = false;
                    }
                    if (bitfield & TILE_FLYOVER_LR) {
                        image.setPixel(px, py + 2,  image.pixel(px, py + 2) | 0xFF00FF00);
                        tileSector->flyable[px][py + 2] = true;
                    } else {
                        tileSector->flyable[px][py + 2] = false;
                    }
                }
            }

            // TODO: We could (or should) save the generated texture for further debugging here
        }

        // Link sectors to neighbouring sectors
        for (uint i = 0; i < count; ++i) {
            TileSector *sector = sectors.data() + i;

            sector->west = 0;
            sector->north = 0;
            sector->south = 0;
            sector->east = 0;

            for (uint j = 0; j < count; ++j) {
                TileSector *other = sectors.data() + j;

                if (other->origin.x() == sector->origin.x() - 192 && other->origin.y() == sector->origin.y())
                    sector->west = other;
                else if (other->origin.x() == sector->origin.x() + 192 && other->origin.y() == sector->origin.y())
                    sector->east = other;
                else if (other->origin.y() == sector->origin.y() - 192 && other->origin.x() == sector->origin.x())
                    sector->north = other;
                else if (other->origin.y() == sector->origin.y() + 192 && other->origin.x() == sector->origin.x())
                    sector->south = other;
            }
        }

        foreach (const Vector4 &startPosition, startPositions) {
            findReachableTiles(startPosition, sectors);
        }

        d->findRectangles(sectors);

        d->findPortals();

        Q_ASSERT(d->validatePortals());

        NavigationMesh *result = new NavigationMesh(d->rectangles, d->portals);
        d->rectangles.clear();
        d->portals.clear();
        return result;
    }

    void NavigationMeshBuilderData::findRectangles(QVector<TileSector> &sectors)
    {
        qDeleteAll(rectangles);
        rectangles.clear();

        for (int si = 0; si < sectors.size(); ++si) {
            TileSector *sector = sectors.data() + si;

            if (!sector->anyReachable) {
                qDebug("Skipping sector %d,%d because it's not reachable from the start position.",
                       sector->origin.x(), sector->origin.y());
            }

            qDebug("Processing sector %d,%d", sector->origin.x(), sector->origin.y());

            QList<QRect> sectorPolygons;
            QRect rect;

            forever {
                // Try to find an unprocessed tile
                for (int x = 0; x < 192; ++x) {
                    for (int y = 0; y < 192; ++y) {
                        if (sector->reachable[x][y]
                            && sector->walkable[x][y]
                            && !sector->used[x][y]
                            && !sector->flyable[x][y]) {
                            rect = QRect(x, y, 1, 1);
                            sector->used[x][y] = true;
                            goto foundTile;
                        }
                    }
                }

                break; // Found no more tiles

                foundTile:

                forever {

                    uint right = rect.x() + rect.width();
                    uint bottom = rect.y() + rect.height();

                    // Try expanding the rectangle by one in every direction
                    while (right < 192) {
                        bool expanded = true;
                        for (uint y = rect.top(); y < bottom; ++y) {
                            if (sector->used[right][y] || !sector->walkable[right][y] || sector->flyable[right][y]) {
                                expanded = false;
                                break;
                            }
                        }

                        if (expanded) {
                            // Mark as used
                            for (uint y = rect.top(); y < bottom; ++y) {
                                sector->used[right][y] = true;
                            }
                            rect.setWidth(rect.width() + 1);
                            right++;
                        } else {
                            break;
                        }
                    }

                    while (bottom < 192) {
                        bool expanded = true;
                        for (uint x = rect.left(); x < right; ++x) {
                            if (sector->used[x][bottom] || !sector->walkable[x][bottom] || sector->flyable[x][bottom]) {
                                expanded = false;
                                break;
                            }
                        }

                        if (expanded) {
                            // Mark as used
                            for (uint x = rect.left(); x < right; ++x) {
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

                sectorPolygons.append(rect);
            }

            bool madeChanges = true;
            int iteration = 0;

            // Second-pass tries to unify more sector polygons
            for (iteration = 0; (iteration < 1000) && madeChanges; ++iteration) {
                madeChanges = false;

                for (int i = 0; i < sectorPolygons.size(); ++i) {
                    for (int j = 0; j < sectorPolygons.size(); ++j) {
                        if(i == j)
                            continue;

                        const QRect &a = sectorPolygons[i];
                        uint leftA = a.x();
                        uint topA = a.y();
                        uint rightA = leftA + a.width();
                        uint bottomA = topA + a.height();

                        uint areaA = (rightA - leftA) * (bottomA - topA);

                        const QRect &b = sectorPolygons[j];
                        uint leftB = b.x();
                        uint topB = b.y();
                        uint rightB = leftB + b.width();
                        uint bottomB = topB + b.height();

                        uint areaB = (rightB - leftB) * (bottomB - topB);

                        // Only consider merging, if the two polygons share an edge
                        SharedEdge edge = getSharedEdge(leftA, topA, rightA, bottomA,
                                                        leftB, topB, rightB, bottomB);

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

                        uint maxArea = qMax(areaA, areaB);

                        if (resultingPrimitives == 1) {
                            leftA = qMin(leftA, leftB);
                            rightA = qMax(rightA, rightB);
                            topA = qMin(topA, topB);
                            bottomA = qMax(bottomA, bottomB);

                            sectorPolygons[i].setCoords(leftA, topA, rightA - 1, bottomA - 1);
                            if (j < i)
                                i--; // Also adjust i for the removed element if it's after j
                            sectorPolygons.removeAt(j--);

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
                                leftA = leftB;
                                if (topB == topA)
                                    topB = bottomA;
                                else if (bottomA == bottomB)
                                    bottomB = topA;
                                else
                                    qFatal("FAIL");
                                break;
                            case Shared_East:
                                rightA = rightB;
                                if (topB == topA)
                                    topB = bottomA;
                                else if (bottomB == bottomA)
                                    bottomB = topA;
                                else
                                    qFatal("FAIL");
                                break;
                            case Shared_North:
                                topA = topB;
                                if (leftB == leftA)
                                    leftB = rightA;
                                else if (rightB == rightA)
                                    rightB = leftA;
                                else
                                    qFatal("FAIL");
                                break;
                            case Shared_South:
                                bottomA = bottomB;
                                if (leftB == leftA)
                                    leftB = rightA;
                                else if (rightB == rightA)
                                    rightB = leftA;
                                else
                                    qFatal("FAIL");
                                break;
                            }

                            uint newAreaA = (rightA - leftA) * (bottomA - topA);
                            uint newAreaB = (rightB - leftB) * (bottomB - topB);
                            Q_ASSERT(newAreaA + newAreaB == areaA + areaB);

                            // Heuristic: Only merge, if it improves the area of the greater of the two rectangles
                            if (newAreaA > maxArea || newAreaB > maxArea) {
                                sectorPolygons[i].setCoords(leftA, topA, rightA - 1, bottomA - 1);
                                sectorPolygons[j].setCoords(leftB, topB, rightB - 1, bottomB - 1);
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
                                sectorPolygons[i].setCoords(leftA, topA, rightA - 1, bottomA - 1);
                                sectorPolygons[j].setCoords(leftB, topB, rightB - 1, bottomB - 1);

                                QRect newRect;
                                newRect.setCoords(leftC, topC, rightC - 1, bottomC - 1);
                                sectorPolygons.append(newRect);
                                madeChanges = true;
                            }
                        }
                    }
                }
            }

            qDebug("Stopped iterating after %d iterations.", iteration);

            foreach (const QRect &sectorRect, sectorPolygons) {
                uint sectorX = sector->origin.x();
                uint sectorY = sector->origin.y();

                NavMeshRect *rect = new NavMeshRect;
                rect->left = (sectorX + sectorRect.left()) * (PixelPerWorldTile / 3);
                rect->right = (sectorX + sectorRect.right() + 1) * (PixelPerWorldTile / 3);
                rect->top = (sectorY + sectorRect.top()) * (PixelPerWorldTile / 3);
                rect->bottom = (sectorY + sectorRect.bottom() + 1) * (PixelPerWorldTile / 3);

                rect->topLeft = Vector4(rect->left, 0, rect->top, 1);
                rect->bottomRight = Vector4(rect->right, 0, rect->bottom, 1);

                rect->center = 0.5f * (rect->topLeft + rect->bottomRight);
                rect->center.setW(1);

                rectangles.append(rect);
            }
        }
    }

    void NavigationMeshBuilderData::findPortals()
    {
        // The rather involved process of finding neighbouring rectangles
        qDeleteAll(portals);
        portals.clear();

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

    bool NavigationMeshBuilderData::validatePortals()
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

}
