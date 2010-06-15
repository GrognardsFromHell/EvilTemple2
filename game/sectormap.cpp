#include "sectormap.h"
#include "texture.h"
#include "scenenode.h"
#include "scene.h"

#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QDataStream>

inline static uint qHash(const QPoint &key) {
    return ((key.x() & 0xFFFF) << 16) | key.y() & 0xFFFF;
}

namespace EvilTemple {

static const float PixelPerWorldTile = 28.2842703f;
static const uint TilesPerSector = 192;

struct Tile {
    uint flags;
    uchar footsteps;
};

class TileSector
{
public:
    TileSector();

    TileSector *west; // x
    TileSector *north; // z-
    TileSector *south; // z-
    TileSector *east; // x+

    QPoint origin;

    void resetVisited();

    bool dirty;

    /*
     The following is used for pathfinding purposes, it replaces
     the closed set and enables constant time lookup in it.
     */
    bool visited[TilesPerSector][TilesPerSector];
    bool walkable[TilesPerSector][TilesPerSector];
    bool flyable[TilesPerSector][TilesPerSector];
};

inline TileSector::TileSector()
    : west(NULL), east(NULL), north(NULL), south(NULL)
{
    resetVisited();
}

inline void TileSector::resetVisited()
{
    dirty = false;
    memset(visited, 0, sizeof(visited));
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

class SectorMapData
{
public:
    Scene *scene;
    int sectorCount;
    QScopedArrayPointer<TileSector> sectors;
    QHash<QPoint, TileSector*> sectorMap;
    QHash<QPoint, bool> offSectorVisited;

    void addSectorToMap(TileSector *sector);
    TileSector *getSector(uint x, uint y);

    void setVisited(const QPoint &p);
    bool isVisited(const QPoint &p);
    bool isWalkable(const QPoint &p);

    void resetVisited();
};

void SectorMapData::addSectorToMap(TileSector *sector)
{
    QPoint sectorId(sector->origin.x() / TilesPerSector, sector->origin.y() / TilesPerSector);
    Q_ASSERT(!sectorMap.contains(sectorId));
    sectorMap[sectorId] = sector;
}

TileSector *SectorMapData::getSector(uint x, uint y)
{
    QPoint sectorId(x / TilesPerSector, y / TilesPerSector);

    QHash<QPoint,TileSector*>::iterator it = sectorMap.find(sectorId);

    if (it == sectorMap.end()) {
        return NULL;
    }

    return it.value();
}

void SectorMapData::resetVisited()
{
    offSectorVisited.clear();

    for (int i = 0; i < sectorCount; ++i) {
        sectors[i].resetVisited();
    }
}

inline void SectorMapData::setVisited(const QPoint &p)
{
    TileSector *sector = getSector(p.x(), p.y());
    if (sector) {
        sector->dirty = true;
        sector->visited[p.x() % 192][p.y() % 192] = true;
    } else {
        offSectorVisited[p] = true;
    }
}

inline bool SectorMapData::isWalkable(const QPoint &p)
{
    TileSector *sector = getSector(p.x(), p.y());
    if (sector) {
        return sector->walkable[p.x() % 192][p.y() % 192];
    } else {
        return false;
    }
}

inline bool SectorMapData::isVisited(const QPoint &p)
{
    TileSector *sector = getSector(p.x(), p.y());
    if (sector) {
        return sector->visited[p.x() % 192][p.y() % 192];
    } else {
        return offSectorVisited[p];
    }
}

SectorMap::SectorMap(Scene *scene) : d(new SectorMapData)
{
    d->scene = scene;
}

SectorMap::~SectorMap()
{
}

bool SectorMap::load(const QString &filename) const
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

    uint secX, secY;

    d->sectors.reset(new TileSector[count]);
    d->sectorCount = count;

    for (int i = 0; i < count; ++i) {
        TileSector *tileSector = d->sectors.data() + i;

        stream >> secY >> secX;

        tileSector->origin = QPoint(secX * 192, secY * 192);

        QImage image(256, 256, QImage::Format_RGB32);
        image.fill(0);

        uchar footstepSound;
        uint bitfield;

/*        image.setPixel(0, 0, qRgba(255, 255, 255, 255));
        image.setPixel(0, 191, qRgba(255, 0, 0, 255));
        image.setPixel(191, 0, qRgba(0, 255, 0, 255)); */

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

        d->addSectorToMap(tileSector);

        SharedTexture texture(new Texture);
        texture->load(image);

        QSharedPointer<Sector> sector(new Sector);
        sector->setTexture(texture);

        SharedSceneNode node(new SceneNode);
        node->setPosition(Vector4(secX * 64 * PixelPerWorldTile, 0, secY * 64 * PixelPerWorldTile, 1));
        node->attachObject(sector);
        d->scene->addNode(node);
    }

    // Link sectors to neighbours
    for (int i = 0; i < d->sectorCount; ++i) {
        TileSector *sector = d->sectors.data() + i;
        for (int j = 0; j < d->sectorCount; ++j) {
            TileSector *other = d->sectors.data() + j;

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
    mTexture->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(renderStates.projectionMatrix().data());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(renderStates.worldViewMatrix().data());

    float d = 191 / 255.f;
    float w = (255 - 191) / 255.f;

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

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glColor4f(0.5, 0.5, 1, 0.5);
    glBegin(GL_LINE_LOOP);
    glVertex3f(0, 0, 0);
    glVertex3f(diagonal.x(), 0, 0);
    glVertex3f(diagonal.x(), 0, diagonal.z());
    glVertex3f(0, 0, diagonal.z());
    glEnd();
}

Sector::Sector()
{
    mBoundingBox = sectorBox;
}

const Box3d &Sector::boundingBox()
{
    return mBoundingBox;
}

inline uint heuristic_estimate_of_distance(const QPoint &start, const QPoint &end)
{
    QPoint dist = (end - start);
    return dist.x() * dist.x() + dist.y() * dist.y();
}

const QPoint directions[8] = {
    QPoint(-1, 0), // West
    QPoint(1, 0), // East
    QPoint(0, -1), // North
    QPoint(0, 1), // South
    QPoint(-1, -1), // Northwest
    QPoint(-1, 1), // Southwest
    QPoint(1, 1), // Southeast
    QPoint(1, -1) // Northeast
};

const uint directionDistance[8] = {
    1, 1, 1, 1,
    2, 2, 2, 2
};

struct OpenWorkItem {
    QPoint p;
    uint f_score;
};

inline bool operator ==(const OpenWorkItem &a, const OpenWorkItem &b) {
    return a.p == b.p;
}

QVector<Vector4> SectorMap::findPath(const Vector4 &start3d, const Vector4 &end3d)
{
    d->resetVisited();

    QPoint start(start3d.x() / (PixelPerWorldTile / 3.0f), start3d.z() / (PixelPerWorldTile / 3.0f));
    QPoint end(end3d.x() / (PixelPerWorldTile / 3.0f), end3d.z() / (PixelPerWorldTile / 3.0f));

    QList<OpenWorkItem> openSet;
    OpenWorkItem startItem;
    startItem.p = start;
    startItem.f_score = heuristic_estimate_of_distance(start, end);
    openSet << startItem;

    QHash<QPoint, uint> g_score, h_score;
    QHash<QPoint, QPoint> cameFrom;
    g_score[start] = 0;
    h_score[start] = heuristic_estimate_of_distance(start, end);

    bool success = false;

    while (!openSet.isEmpty()) {
        OpenWorkItem owi = openSet.takeFirst();
        QPoint x = owi.p;

        if (x == end) {
            success = true;
            break;
        }

        d->setVisited(x);

        // Add all neighbour nodes (no diagonals) (shoudl be unrolled)
        for (int i = 0; i < 8; ++i) {
            QPoint y = x + directions[i];

            if (!d->isWalkable(y))
                continue;

            if (d->isVisited(y))
                continue;

            uint tentative_g_score = g_score[x] + directionDistance[i]; // dist_between(x,y)

            bool tentativeIsBetter = false;
            uint goalDistance = heuristic_estimate_of_distance(y, end);
            uint y_f_score = tentative_g_score + goalDistance;

            OpenWorkItem wi;
            wi.p = y;
            wi.f_score = y_f_score;

            // This is expensive, somehow possible to optimize?
            int index = openSet.indexOf(wi);

            if (index == -1) {
                // Do insertion sort
                bool inserted = false;
                for (int j = 0; j < openSet.size(); ++j) {
                    uint fscore = openSet[j].f_score;
                    if (fscore >= y_f_score) {
                        openSet.insert(j, wi);
                        inserted = true;
                        break;
                    }
                }
                if (!inserted)
                    openSet.append(wi);
                tentativeIsBetter = true;
            } else if (tentative_g_score < g_score[y]) {
                tentativeIsBetter = true;
                openSet[index].f_score = y_f_score;
            }

            if (tentativeIsBetter) {
                cameFrom[y] = x;
                g_score[y] = tentative_g_score;                
                h_score[y] = goalDistance;
            }
        }
    }

    if (success) {
        Q_ASSERT(cameFrom.contains(end));

        QVector<Vector4> result;

        // Reconstructing the result set

        QPoint current = end;

        do {
            result.append(Vector4(current.x() * PixelPerWorldTile / 3, 0, current.y() * PixelPerWorldTile / 3, 1));
            current = cameFrom[current];
        } while (!current.isNull());

        result << start3d << end3d;

        return result;
    } else {
        return QVector<Vector4>();
    }

}

}
