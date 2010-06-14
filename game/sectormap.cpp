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

SectorMap::SectorMap(Scene *scene) : mScene(scene)
{
}

static const float PixelPerWorldTile = 28.2842703f;

struct Tile {
    uint flags;
    uchar footsteps;
};

struct TileSector {
    TileSector *west; // x
    TileSector *north; // z-
    TileSector *south; // z-
    TileSector *east; // x+

    QPoint origin;

    bool used[192][192];
    bool visited[192][192];
    bool walkable[192][192];
    bool flyable[192][192];
};

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

    QList<TileSector*> sectors;

    for (int i = 0; i < count; ++i) {

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

        image.setPixel(0, 0, qRgba(255, 255, 255, 255));
        image.setPixel(0, 191, qRgba(255, 0, 0, 255));
        image.setPixel(191, 0, qRgba(0, 255, 0, 255));

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
        mScene->addNode(node);
    }

    // Link sectors to neighbours
    foreach (TileSector *sector, sectors) {
        sector->west = 0;
        sector->north = 0;
        sector->south = 0;
        sector->east = 0;

        foreach (TileSector *other, sectors) {
            if (other->origin.x() == sector->origin.x() - 1 && other->origin.y() == sector->origin.y())
                sector->west = other;
            else if (other->origin.x() == sector->origin.x() + 1 && other->origin.y() == sector->origin.y())
                sector->east = other;
            else if (other->origin.y() == sector->origin.y() - 1 && other->origin.x() == sector->origin.x())
                sector->north = other;
            else if (other->origin.y() == sector->origin.y() + 1 && other->origin.x() == sector->origin.x())
                sector->south = other;
        }
    }
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

}
