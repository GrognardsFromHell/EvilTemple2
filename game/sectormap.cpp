#include "sectormap.h"
#include "texture.h"
#include "scenenode.h"
#include "scene.h"

#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QDataStream>
#include <QElapsedTimer>

inline static uint qHash(const QPoint &key) {
    return ((key.x() & 0xFFFF) << 16) | key.y() & 0xFFFF;
}

namespace EvilTemple {


inline bool westeast_intersect(int y, int left, int right, const QPoint &from, const QPoint &to)
{
    // Parallel to the axis -> reject
    if (from.y() == to.y())
        return false;

    if (left > qMax(from.x(), to.x()))
        return false;

    if (right < qMin(from.x(), to.x()))
        return false;

    int ydiff = y - from.y();
    float xascent = (to.x() - from.x()) / (float)(to.y() - from.y());

    float ix = from.x() + xascent * ydiff;

    return ix >= left && ix <= right;
}

inline bool northsouth_intersect(int x, int top, int bottom, const QPoint &from, const QPoint &to)
{
    // Parallel to the axis -> reject
    if (from.x() == to.x())
        return false;

    if (top > qMax(from.y(), to.y()))
        return false;

    if (bottom < qMin(from.y(), to.y()))
        return false;

    int xdiff = x - from.x();
    float yascent = (to.y() - from.y()) / (float)(to.x() - from.x());

    float iy = from.y() + yascent * xdiff;

    return iy >= top && iy <= bottom;
}

#define SAME_SIGNS( a, b )	\
    (((long) ((unsigned long) a ^ (unsigned long) b)) >= 0 )


#define	DONT_INTERSECT    0
#define	DO_INTERSECT      1
#define COLLINEAR         2

            int lines_intersect( float x1, float y1,   /* First line segment */
                                 float x2, float y2,

                                 float x3, float y3,   /* Second line segment */
                                 float x4, float y4,

                                 int *x,
                                 int *y         /* Output value:
                                * point of intersection */
                                 )
    {
        float a1, a2, b1, b2, c1, c2; /* Coefficients of line eqns. */
        float r1, r2, r3, r4;         /* 'Sign' values */
        float denom, offset, num;     /* Intermediate values */

        /* Compute a1, b1, c1, where line joining points 1 and 2
     * is "a1 x  +  b1 y  +  c1  =  0".
     */

        a1 = y2 - y1;
        b1 = x1 - x2;
        c1 = x2 * y1 - x1 * y2;

        /* Compute r3 and r4.
     */


        r3 = a1 * x3 + b1 * y3 + c1;
        r4 = a1 * x4 + b1 * y4 + c1;

        /* Check signs of r3 and r4.  If both point 3 and point 4 lie on
     * same side of line 1, the line segments do not intersect.
     */

        if ( r3 != 0 &&
             r4 != 0 &&
             SAME_SIGNS( r3, r4 ))
            return ( DONT_INTERSECT );

        /* Compute a2, b2, c2 */

        a2 = y4 - y3;
        b2 = x3 - x4;
        c2 = x4 * y3 - x3 * y4;

        /* Compute r1 and r2 */

        r1 = a2 * x1 + b2 * y1 + c2;
        r2 = a2 * x2 + b2 * y2 + c2;

        /* Check signs of r1 and r2.  If both point 1 and point 2 lie
     * on same side of second line segment, the line segments do
     * not intersect.
     */

        if ( r1 != 0 &&
             r2 != 0 &&
             SAME_SIGNS( r1, r2 ))
            return ( DONT_INTERSECT );

        /* Line segments intersect: compute intersection point.
     */

        denom = a1 * b2 - a2 * b1;
        if ( denom == 0 )
            return ( COLLINEAR );
        offset = denom < 0 ? - denom / 2 : denom / 2;

        /* The denom/2 is to get rounding instead of truncating.  It
     * is added or subtracted to the numerator, depending upon the
     * sign of the numerator.
     */

        num = b1 * c2 - b2 * c1;
        *x = ( num < 0 ? num - offset : num + offset ) / denom;

        num = a2 * c1 - a1 * c2;
        *y = ( num < 0 ? num - offset : num + offset ) / denom;

        return ( DO_INTERSECT );
    } /* lines_intersect */

    static const float PixelPerWorldTile = 28.2842703f;

    struct Tile {
        uint flags;
        uchar footsteps;
    };

    struct NavMeshPortal;

    class NavMeshRect {
    public:
        // Global coordinates
        uint left;
        uint top;
        uint right;
        uint bottom;

        uint centerX;
        uint centerY;

        QVector<const NavMeshPortal*> portals;
    };

    enum PortalAxis {
        NorthSouth,
        WestEast
    };

    struct NavMeshPortal {
        NavMeshRect *sideA;
        NavMeshRect *sideB;

        PortalAxis axis; // The axis on which this portal lies
        uint start, end; // The start and end of the portal on the given axis

        // Position of the middle of this portal
        float x;
        float y;
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

    /**
  Calculates the area contained in the given, simple polygon.
  */
    inline static float area(const QPolygon &polygon)
    {
        int result = 0;

        // TODO: Optimize so this doesn't access all points twice.
        int x, y, nx, ny;
        for (int i = 0; i < polygon.size() - 1; ++i) {
            polygon.point(i, &x, &y);
            polygon.point(i, &nx, &ny);
            result += x * ny + nx * y;
        }

        // Loop back to the start
        if (polygon.size() > 1) {
            polygon.point(0, &x, &y);
            result += x * ny + nx * y;
        }

        if (result < 0)
            result = - result;

        return result * 0.5f;
    }

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

    class SectorMapData
    {
    public:
        Scene *scene;
        QWeakPointer<Sector> sector;

        QVector<NavMeshRect*> rectangles;
        QVector<NavMeshPortal*> portals;

        const NavMeshRect *findRect(const Vector4 &position) const;
        void clearExistingSector();
    };

    SectorMap::SectorMap(Scene *scene) : d(new SectorMapData)
    {
        d->scene = scene;
    }

    SectorMap::~SectorMap()
    {
    }

    void SectorMapData::clearExistingSector()
    {
        QSharedPointer<Sector> sec = sector;
        sector.clear();

        if (sec) {
            sec->clearNavMeshRects();
        }
    }

    const NavMeshRect *SectorMapData::findRect(const Vector4 &position) const
    {
        int x = position.x() / (PixelPerWorldTile / 3);
        int y = position.z() / (PixelPerWorldTile / 3);

        for (int i = 0; i < rectangles.size(); ++i) {
            NavMeshRect *rect = rectangles[i];

            if (x >= rect->left && x <= rect->right && y >= rect->top && y <= rect->bottom) {
                return rect;
            }
        }

        return NULL;
    }

    inline QPoint vectorToPoint(const Vector4 &vector)
    {
        static const float factor = 1 / (PixelPerWorldTile / 3.0f);
        return QPoint(vector.x() * factor, vector.z() * factor);
    }

    static TileSector *findSector(const QList<TileSector*> &sectors, const QPoint &pos)
    {
        foreach (TileSector *sector, sectors) {
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

    static void findReachableTiles(const Vector4 &startPosition, const QList<TileSector*> &sectors)
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

    bool SectorMap::load(const Vector4 &startPosition, const QString &filename) const
    {
        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }

        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        uint count;
        stream >> count;

        if (count == 0)
            return true;

        // Clear the existing sector if it still exists
        d->clearExistingSector();

        uint secX, secY;

        QList<TileSector*> sectors;

        for (uint i = 0; i < count; ++i) {

            TileSector *tileSector = new TileSector;
            memset(tileSector->visited, 0, sizeof(tileSector->visited));
            memset(tileSector->used, 0, sizeof(tileSector->used));
            sectors.append(tileSector);

            stream >> secY >> secX;

            tileSector->origin = QPoint(secX * 192, secY * 192);

            QImage image(256, 256, QImage::Format_RGB32);
            image.fill(0);

            uchar footstepSound;
            uint bitfield;

            /*image.setPixel(0, 0, qRgba(255, 255, 255, 255));
        image.setPixel(0, 191, qRgba(255, 0, 0, 255));
        image.setPixel(191, 0, qRgba(0, 255, 0, 255));*/

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

            SharedTexture texture(new Texture);
            texture->load(image);

            QSharedPointer<Sector> sector(new Sector);
            sector->setTexture(texture);

            SharedSceneNode node(new SceneNode);
            node->setPosition(Vector4(secX * 64 * PixelPerWorldTile, 0, secY * 64 * PixelPerWorldTile, 1));
            node->attachObject(sector);
            d->scene->addNode(node);
        }

        // Link sectors to neighbouring sectors
        foreach (TileSector *sector, sectors) {
            sector->west = 0;
            sector->north = 0;
            sector->south = 0;
            sector->east = 0;

            foreach (TileSector *other, sectors) {
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

        findReachableTiles(startPosition, sectors);

        QList<QRect> polygons;
        // There can be a *lot* of rectangles, but we estimate that we can remove 80% through merging
        polygons.reserve(192 * 192 * sectors.size() * 0.2);

        foreach (TileSector *sector, sectors) {

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

            foreach (QRect rect, sectorPolygons) {
                rect.setTopLeft(rect.topLeft() + sector->origin);
                rect.setBottomRight(rect.bottomRight() + sector->origin);
                polygons.append(rect);
            }
        }

        if (!polygons.isEmpty()) {
            qDebug("Adding %d polygons.", polygons.size());
            Sector *sec = new Sector;

            uint i = 0;
            QHash<QPoint, uint> verts;
            QStringList vertices;
            QStringList faces;

            static const float scale = 0.1f;

            qDeleteAll(d->rectangles);
            d->rectangles.clear();
            d->rectangles.reserve(polygons.size());

            for (int j = 0; j < polygons.size(); ++j) {
                const QRect &polygon = polygons[j];

                NavMeshRect *rect = new NavMeshRect;
                d->rectangles.append(rect);
                rect->left = polygon.left();
                rect->right = polygon.right() + 1;
                rect->top = polygon.top();
                rect->bottom = polygon.bottom() + 1;

                rect->centerX = (rect->right + rect->left) / 2;
                rect->centerY = (rect->top + rect->bottom) / 2;

                sec->addNavMeshRect(rect);

                QPoint topLeft = polygon.topLeft();
                uint topLeftIndex;
                QPoint topRight = polygon.topRight();
                topRight.setX(topRight.x() + 1);
                uint topRightIndex;
                QPoint bottomLeft = polygon.bottomLeft();
                bottomLeft.setY(bottomLeft.y() + 1);
                uint bottomLeftIndex;
                QPoint bottomRight = polygon.bottomRight();
                bottomRight.setY(bottomRight.y() + 1);
                bottomRight.setX(bottomRight.x() + 1);
                uint bottomRightIndex;

                if (verts.contains(topLeft))
                    topLeftIndex = verts[topLeft];
                else {
                    verts[topLeft] = ++i;
                    topLeftIndex = i;
                    float x = topLeft.x();
                    float y = topLeft.y();
                    vertices.append(QString("v %1 0 %2").arg(x * scale).arg(y * scale));
                }

                if (verts.contains(bottomLeft))
                    bottomLeftIndex = verts[bottomLeft];
                else {
                    verts[bottomLeft] = ++i;
                    bottomLeftIndex = i;
                    float x = bottomLeft.x();
                    float y = bottomLeft.y();
                    vertices.append(QString("v %1 0 %2").arg(x * scale).arg(y * scale));
                }

                if (verts.contains(bottomRight))
                    bottomRightIndex = verts[bottomRight];
                else {
                    verts[bottomRight] = ++i;
                    bottomRightIndex = i;
                    float x = bottomRight.x();
                    float y = bottomRight.y();
                    vertices.append(QString("v %1 0 %2").arg(x * scale).arg(y * scale));
                }

                if (verts.contains(topRight))
                    topRightIndex = verts[topRight];
                else {
                    verts[topRight] = ++i;
                    topRightIndex = i;
                    float x = topRight.x();
                    float y = topRight.y();
                    vertices.append(QString("v %1 0 %2").arg(x * scale).arg(y * scale));
                }

                faces.append(QString("f %1 %2 %3 %4").arg(topRightIndex).arg(topLeftIndex).arg(bottomLeftIndex).arg(bottomRightIndex));
            }

            QFile test("test.obj");
            test.open(QIODevice::WriteOnly|QIODevice::Text);
            QTextStream s(&test);

            foreach (QString v, vertices) {
                s << v << endl;
            }
            foreach (QString f, faces) {
                s << f << endl;
            }
            s.flush();

            // The rather involved process of finding neighbouring rectangles
            qDeleteAll(d->portals);
            d->portals.clear();

            for (int i = 0; i < d->rectangles.size(); ++i) {
                NavMeshRect *rect = d->rectangles.at(i);

                for (int j = 0; j < d->rectangles.size(); ++j) {
                    if (i == j)
                        continue;

                    NavMeshRect *other = d->rectangles.at(j);

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
                    d->portals.append(portal);
                    portal->sideA = rect;
                    portal->sideB = other;

                    switch (edge) {
                    case Shared_North:
                        portal->start = qMax(rect->left, other->left);
                        portal->end = qMin(rect->right, other->right);
                        portal->x = 0.5f * (portal->start + portal->end);
                        portal->y = rect->top;                        
                        portal->axis = WestEast;
                        break;
                    case Shared_South:
                        portal->start = qMax(rect->left, other->left);
                        portal->end = qMin(rect->right, other->right);
                        portal->x = 0.5f * (portal->start + portal->end);
                        portal->y = rect->bottom;
                        portal->axis = WestEast;
                        break;
                    case Shared_East:
                        portal->start = qMax(rect->top, other->top);
                        portal->end = qMin(rect->bottom, other->bottom);
                        portal->x = rect->right;
                        portal->y = 0.5f * (portal->start + portal->end);
                        portal->axis = NorthSouth;
                        break;
                    case Shared_West:                        
                        portal->start = qMax(rect->top, other->top);
                        portal->end = qMin(rect->bottom, other->bottom);
                        portal->x = rect->left;
                        portal->y = 0.5f * (portal->start + portal->end);
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

            // Check some invariants for all portals
            foreach (const NavMeshPortal *portal, d->portals) {
                if (portal->sideA == portal->sideB) {
                    qFatal("Looping portal detected.");
                }
                if (portal->axis == NorthSouth) {
                    if (portal->x - floor(portal->x) != 0) {
                        qFatal("Portal on x north-south axis with non-integral X component.");
                    }
                } else if (portal->axis == WestEast) {
                    if (portal->y - floor(portal->y) != 0) {
                        qFatal("Portal on x north-south axis with non-integral X component.");
                    }
                } else {
                    qFatal("Portal with invalid axis.");
                }

                const NavMeshRect *sideA = portal->sideA;
                const NavMeshRect *sideB = portal->sideB;

                // Check that the two linked rectangles even share the expected edge
                if (portal->axis == NorthSouth) {
                    if (sideA->left == sideB->right || sideA->right == sideB->left) {
                        if (sideA->top >= sideB->bottom || sideA->bottom <= sideB->top) {
                            qFatal("Two linked rectangles aren't touching!");
                        }
                    } else {
                        qFatal("North-South axis portal links two rectangles that don't touch.");
                    }
                } else if (portal->axis == WestEast) {
                    if (sideA->top == sideB->bottom || sideA->bottom == sideB->top) {
                        if (sideA->left >= sideB->right || sideA->right <= sideB->left) {
                            qFatal("Two linked rectangles aren't touching!");
                        }
                    } else {
                        qFatal("West-East axis portal links two rectangles that don't touch.");
                    }
                }

                // Assert that both sideA and sideB are in the list of rectangles
                bool foundA = false, foundB = false;
                foreach (const NavMeshRect *rect, d->rectangles) {
                    if (rect == sideA)
                        foundA = true;
                    if (rect == sideB)
                        foundB = true;
                }

                if (!foundA || !foundB) {
                    qFatal("Found portal with disconnected sides.");
                }
            }                      

            QSharedPointer<Sector> renderable(sec);

            d->sector = renderable;

            SceneNode *sceneNode = new SceneNode();
            sceneNode->setPosition(Vector4(0,0,0,1));
            sceneNode->attachObject(renderable);

            d->scene->addNode(SharedSceneNode(sceneNode));
        }

        qDeleteAll(sectors);
        return true;
    }


    static const Vector4 diagonal(64 * PixelPerWorldTile, 0, 64 * PixelPerWorldTile, 1);
    static const Box3d sectorBox(Vector4(0,0,0,1), diagonal);

    void Sector::render(RenderStates &renderStates)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glColor4f(1, 1, 1, 0.25);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_TEXTURE_2D);
        if (mTexture)
            mTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(renderStates.projectionMatrix().data());
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(renderStates.worldViewMatrix().data());
        /*

    float d = 191 / 255.f;
    float w = (255 - 191) / 255.f;
    if (mTexture)
    {
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1);
        glVertex3f(0, 0, 0);
        glTexCoord2f(d, 1);
        glVertex3f(diagonal.x(), 0, 0);
        glTexCoord2f(d, w);
        glVertex3f(diagonal.x(), 0, diagonal.z());
        glTexCoord2f(0, w);
        glVertex3f(0, 0, diagonal.z());
        glEnd();
    }*/

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        glColor4f(0.5, 0.5, 1, 0.5);
        glBegin(GL_LINE_LOOP);
        glVertex3f(0, 0, 0);
        glVertex3f(diagonal.x(), 0, 0);
        glVertex3f(diagonal.x(), 0, diagonal.z());
        glVertex3f(0, 0, diagonal.z());
        glEnd();

        srand(1234656812);

        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        foreach (const NavMeshRect *rect, mNavMeshRects) {
            float r = 0.5f + 0.5f * (rand() / (float)RAND_MAX);
            float g = 0.5f + 0.5f * (rand() / (float)RAND_MAX);
            float b = 0.5f + 0.5f * (rand() / (float)RAND_MAX);
            glColor4f(r, g, b, 0.5);

            float left = rect->left / 3.0f * PixelPerWorldTile;
            float top = rect->top / 3.0f * PixelPerWorldTile;
            float right = rect->right / 3.0f * PixelPerWorldTile;
            float bottom = rect->bottom / 3.0f * PixelPerWorldTile;

            glVertex3f(left, 0, top);
            glVertex3f(left, 0, bottom);
            glVertex3f(right, 0, bottom);
            glVertex3f(right, 0, top);
        }
        glEnd();

        // Go over all nodes again and draw a cross over all nodes that have been traversed
        glLineWidth(2);
        glBegin(GL_LINES);
        foreach (const NavMeshRect *rect, mNavMeshRects) {
            float left = rect->left / 3.0f * PixelPerWorldTile;
            float top = rect->top / 3.0f * PixelPerWorldTile;
            float right = rect->right / 3.0f * PixelPerWorldTile;
            float bottom = rect->bottom / 3.0f * PixelPerWorldTile;
            glColor3f(0, 0.8f, 0);

            /*if (rect->inClosedSet || rect->partOfPath) {
                glVertex3f(left, 0, top);
                glVertex3f(right, 0, top);
                glVertex3f(right, 0, top);
                glVertex3f(right, 0, bottom);
                glVertex3f(right, 0, bottom);
                glVertex3f(left, 0, bottom);
                glVertex3f(left, 0, bottom);
                glVertex3f(left, 0, top);
            }

            if (rect->inClosedSet && !rect->partOfPath) {
                glVertex3f(left, 0, top);
                glVertex3f(right, 0, bottom);
                glVertex3f(left, 0, bottom);
                glVertex3f(right, 0, top);
            }*/
        }
        glEnd();

        glPointSize(2);
        glBegin(GL_POINTS);
        foreach (const NavMeshRect *rect, mNavMeshRects) {
            glColor3f(0.1, 0.1f, 0.8f);

            foreach (const NavMeshPortal *portal, rect->portals) {
                if (portal->sideA != rect)
                    continue; // Only draw one side, some mesh is going to own the other side

                float x = portal->x / 3.0f * PixelPerWorldTile;
                float y = portal->y / 3.0f * PixelPerWorldTile;
                glVertex3f(x, 0, y);
            }
        }
        glEnd();

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }

    Sector::Sector()
    {
        mBoundingBox = sectorBox;
        mRenderCategory = RenderQueue::DebugOverlay;
    }

    const Box3d &Sector::boundingBox()
    {
        return mBoundingBox;
    }

    void Sector::addNavMeshRect(const NavMeshRect *rect)
    {
        float f = 1 / 3.0f * PixelPerWorldTile;
        Vector4 topLeft(rect->left, 0, rect->top, 1);
        mBoundingBox.merge(f * topLeft);
        Vector4 bottomRight  = Vector4(rect->right, 0, rect->bottom, 1);
        mBoundingBox.merge(f * bottomRight);

        mNavMeshRects.append(rect);
    }

    inline Vector4 vectorFromPoint(uint x, uint y)
    {
        return Vector4(x * (PixelPerWorldTile / 3), 0, y * (PixelPerWorldTile / 3), 1);
    }

    inline uint getDistanceHeuristic(const NavMeshPortal *portal, const QPoint &point) {
        float diffx = portal->x - point.x();
        float diffy = portal->y - point.y();
        return sqrt(diffx * diffx + diffy * diffy);
    }

    inline uint getTraversalCost(const NavMeshPortal *portalA, const NavMeshPortal *portalB) {
        float diffx = portalA->x - portalB->x;
        float diffy = portalA->y - portalB->y;
        return sqrt(diffx * diffx + diffy * diffy);
    }

    /**
      Node markings used by AStar
      */
    struct AStarNode {
        const NavMeshPortal *portal;
        AStarNode *parent;
        /*
         Means that the portal we're coming from is in sideA of portal. Used to avoid adding unnecessary nodes
         to the open set. For starting portals, this is true if the starting node is in the rect denoted by sideA.
         */
        bool comingFromSideA;

        bool inClosedSet;
        bool inOpenSet;
        uint costFromStart;
        uint costToGoal;
        uint totalCost;

        // Debugging flags
        bool partOfPath;
    };

    inline uint getDistanceHeuristic(const AStarNode *node, const QPoint &point) {

        int x, y;

        if (node->comingFromSideA) {
            x = node->portal->sideB->centerX;
            y = node->portal->sideB->centerY;
        } else {
            x = node->portal->sideA->centerX;
            y = node->portal->sideA->centerY;
        }

        float diffx = x - point.x();
        float diffy = y - point.y();
        return sqrt(diffx * diffx + diffy * diffy);
    }

    bool compareAStarNodes(const AStarNode *a, const AStarNode *b)
    {
        return a->totalCost < b->totalCost;
    }

    bool checkLos(const NavMeshRect *losStartRect, const QPoint &losStart, const QPoint &losEnd)
    {
        const NavMeshRect *currentRect = losStartRect;
        const NavMeshRect *gotInFrom = NULL;

        QVector<const NavMeshRect*> visited;

        // Check with each of the current rects portals, whether the line intersects
        forever {
            // If the end point lies in the current rectangle, we succeeded
            if (losEnd.x() >= currentRect->left && losEnd.x() <= currentRect->right
                && losEnd.y() >= currentRect->top && losEnd.y() <= currentRect->bottom)
            {
                // TODO: Take dynamic LOS into account?
                return true;
            }

            bool foundPortal = false;

            foreach (const NavMeshPortal *portal, currentRect->portals) {
                if (visited.contains(portal->sideA) || visited.contains(portal->sideB))
                    continue; // Prevent going back

                // There are only two axes here, so an if-else will suffice
                if (portal->axis == NorthSouth) {
                    if (!northsouth_intersect(portal->x, portal->start, portal->end, losStart, losEnd))
                        continue;
                } else {
                    if (!westeast_intersect(portal->y, portal->start, portal->end, losStart, losEnd))
                        continue;
                }

                // Take this portal
                gotInFrom = currentRect;
                visited << currentRect;
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

    QVector<Vector4> SectorMap::findPath(const Vector4 &start, const Vector4 &end) const
    {
        QElapsedTimer timer;
        timer.start();

        // Find first and last navmesh tiles
        const NavMeshRect *startRect = d->findRect(start);
        const NavMeshRect *endRect = d->findRect(end);
        QVector<Vector4> result;

        if (!startRect || !endRect) {
            qDebug("Either end or startpoint is not walkable. Time taken: %d ms", (int)timer.elapsed());

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

        QPoint startPos(start.x() / (PixelPerWorldTile / 3), start.z() / (PixelPerWorldTile / 3));
        QPoint endPos(end.x() / (PixelPerWorldTile / 3), end.z() / (PixelPerWorldTile / 3));

        QList<AStarNode*> openSet;
        QHash<const NavMeshPortal*, AStarNode*> portalState;

        // Add all the portals accessible from the start position with correct cost
        for (int i = 0; i < startRect->portals.size(); ++i) {
            const NavMeshPortal *portal = startRect->portals[i];

            AStarNode *node = new AStarNode;
            node->portal = portal;
            node->parent = NULL;
            node->inOpenSet = true;
            node->inClosedSet = false;
            node->comingFromSideA = portal->sideA == startRect;
            node->costFromStart = getDistanceHeuristic(portal, startPos);
            node->costToGoal = getDistanceHeuristic(node, endPos);
            node->totalCost = node->costFromStart + node->costToGoal;

            // Map portal to node
            portalState[portal] = node;

            openSet.append(node);
        }

        qSort(openSet.begin(), openSet.end(), compareAStarNodes);

        int touchedNodes = 0;
        AStarNode *lastNode = NULL;

        while (!openSet.isEmpty()) {
            AStarNode *node = openSet.takeFirst();
            node->inOpenSet = false;

            touchedNodes++;

            if (node->portal->sideA == endRect || node->portal->sideB == endRect) {
                lastNode = node;
                break; // Reached end successfully.
            }

            uint currentCost = node->costFromStart;

            const NavMeshRect *traverseInto = node->comingFromSideA ? node->portal->sideB : node->portal->sideA;

            bool openSetChanged = false;

            for (int i = 0; i < traverseInto->portals.size(); ++i) {
                const NavMeshPortal *portal = traverseInto->portals[i];

                if (portal == node->portal)
                    continue; // don't add the portal we're coming from

                uint neighbourCost = currentCost + getTraversalCost(node->portal, portal);

                // TODO: Try finding an AStar node for the portal
                QHash<const NavMeshPortal*,AStarNode*>::iterator it = portalState.find(portal);

                AStarNode *otherNode;

                if (it != portalState.end()) {
                    otherNode = it.value();

                    if ((otherNode->inClosedSet || otherNode->inOpenSet)
                        && otherNode->costFromStart <= neighbourCost)
                        continue; // Skip, we have already a better path to this node
                } else {
                    otherNode = new AStarNode;
                    portalState[portal] = otherNode;                    
                }

                otherNode->comingFromSideA = portal->sideA == traverseInto;
                otherNode->portal = portal;
                otherNode->parent = node;
                otherNode->costFromStart = neighbourCost;
                otherNode->costToGoal = getDistanceHeuristic(otherNode, endPos);
                otherNode->totalCost = otherNode->costFromStart + otherNode->costToGoal;
                otherNode->inOpenSet = true;
                otherNode->inClosedSet = false;

                openSet.append(otherNode);
                openSetChanged = true;
            }

            if (openSetChanged)
                qSort(openSet.begin(), openSet.end(), compareAStarNodes);

            node->inClosedSet = true;
        }

        if (lastNode) {
            QVector<QPoint> intermediatePath;
            QVector<const NavMeshRect*> intermediateRects;

            intermediatePath << endPos;
            intermediateRects << endRect;

            AStarNode *node = lastNode;
            while (node) {
                const NavMeshPortal *portal = node->portal;
                intermediatePath.prepend(QPoint(portal->x, portal->y));
                if (node->comingFromSideA)
                    intermediateRects.prepend(portal->sideB);
                else
                    intermediateRects.prepend(portal->sideA);
                node = node->parent;
            }

            intermediatePath.prepend(startPos);
            intermediateRects.prepend(startRect);

            qDebug() << "A* finished after" << timer.restart() << "ms.";

            // Try compressing the path through LOS checks
            for (int i = 0; i < intermediatePath.size() - 1; ++i) {
                QPoint losStart = intermediatePath[i];
                const NavMeshRect *losStartRect = intermediateRects[i];

                for (int j = intermediatePath.size() - 1; j > i + 1; --j) {
                    QPoint losEnd = intermediatePath[j];

                    if (checkLos(losStartRect, losStart, losEnd)) {
                        // Remove all nodes between losStart and losEnd from the intermediate path
                        intermediatePath.remove(i + 1, j - (i + 1));
                        intermediateRects.remove(i + 1, j - (i + 1));
                        break;
                    }
                }
            }

            for (int i = 0; i < intermediatePath.size(); ++i) {
                result.append(vectorFromPoint(intermediatePath[i].x(), intermediatePath[i].y()));
            }
        }

        qDeleteAll(portalState.values()); // Should use object pooling instead

        qint64 elapsed = timer.elapsed();

        qDebug() << "Pathfinding done in" << elapsed << "ms. Nodes visited:"
                << touchedNodes << "Success:" << (!result.isEmpty() ? "yes" : "no");

        return result;
    }

    bool SectorMap::hasLineOfSight(const Vector4 &from, const Vector4 &to) const
    {
        const NavMeshRect *startRect = d->findRect(from);

        if (!startRect || !d->findRect(to))
            return false;

        QPoint fromPoint = vectorToPoint(from);
        QPoint toPoint = vectorToPoint(to);

        return checkLos(startRect, fromPoint, toPoint);
    }

}
