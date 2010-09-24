
#include <QScopedPointer>
#include <QFile>
#include <QDataStream>

#include <common/quadtree.h>

#include "conversion/sectorconverter.h"

using namespace Troika;

BoolQuadtree *createQuadtreeWalkable(const ZoneTemplate *zoneTemplate);
BoolQuadtree *createQuadtreeFlyable(const ZoneTemplate *zoneTemplate);
StringQuadtree *createGroundMaterialQuadtree(const ZoneTemplate *zoneTemplate);
CharQuadtree *createHeightQuadtree(const ZoneTemplate *zoneTemplate);
BoolQuadtree *createVisionExtendQuadtree(const ZoneTemplate *zoneTemplate);
BoolQuadtree *createVisionBaseQuadtree(const ZoneTemplate *zoneTemplate);
BoolQuadtree *createVisionArchwayQuadtree(const ZoneTemplate *zoneTemplate);
BoolQuadtree *createVisionEndQuadtree(const ZoneTemplate *zoneTemplate);

SectorConverter::SectorConverter(const ZoneTemplate *zoneTemplate)
    : mZoneTemplate(zoneTemplate)
{
}

QByteArray SectorConverter::convert()
{
    QScopedPointer<BoolQuadtree> walkableLayer(createQuadtreeWalkable(mZoneTemplate));
    QScopedPointer<BoolQuadtree> flyableLayer(createQuadtreeFlyable(mZoneTemplate));
    QScopedPointer<StringQuadtree> groundMaterialLayer(createGroundMaterialQuadtree(mZoneTemplate));
    QScopedPointer<CharQuadtree> heightLayer(createHeightQuadtree(mZoneTemplate));
    QScopedPointer<BoolQuadtree> visionExtendLayer(createVisionExtendQuadtree(mZoneTemplate));
    QScopedPointer<BoolQuadtree> visionEndLayer(createVisionEndQuadtree(mZoneTemplate));
    QScopedPointer<BoolQuadtree> visionBaseLayer(createVisionBaseQuadtree(mZoneTemplate));
    QScopedPointer<BoolQuadtree> visionArchwayLayer(createVisionArchwayQuadtree(mZoneTemplate));

    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << *walkableLayer.data() << *flyableLayer.data() << *groundMaterialLayer.data()
            << *heightLayer.data() << *visionExtendLayer.data() << *visionEndLayer.data()
            << *visionBaseLayer.data() << *visionArchwayLayer.data();

    return result;
}

QImage SectorConverter::createWalkableImage() const
{
    QScopedPointer<BoolQuadtree> tree(createQuadtreeWalkable(mZoneTemplate));

    QFile file("test.dat");
    file.open(QIODevice::Truncate|QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << *tree.data();
    file.close();

    file.open(QIODevice::ReadOnly);
    QDataStream streamIn(&file);
    streamIn.setByteOrder(QDataStream::LittleEndian);
    streamIn >> *tree.data();
    file.close();

    QImage result(4096, 4096, QImage::Format_ARGB32);

    result.fill(0);

    for (int x = 0; x < 4096; ++x) {
        for (int y = 0; y < 4096; ++y) {
            bool walkable = tree->get(x, y);

            if (walkable)
                result.setPixel(x, y, qRgb(0, 255, 0));
        }
    }

    return result;
}

QImage SectorConverter::createGroundMaterialImage() const
{
    QScopedPointer<StringQuadtree> tree(createGroundMaterialQuadtree(mZoneTemplate));

    QFile file("test.dat");
    file.open(QIODevice::Truncate|QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << *tree.data();
    file.close();

    file.open(QIODevice::ReadOnly);
    QDataStream streamIn(&file);
    streamIn.setByteOrder(QDataStream::LittleEndian);
    streamIn >> *tree.data();
    file.close();

    QImage result(4096, 4096, QImage::Format_ARGB32);

    result.fill(0);

    QHash<QString, uint> colorMap;
    colorMap["dirt"] = qRgb(89, 82, 49);
    colorMap["grass"] = qRgb(98, 164, 50);
    colorMap["water"] = qRgb(44, 89, 255);
    colorMap["deepWater"] = qRgb(32, 57, 153);
    colorMap["ice"] = qRgb(132, 251, 255);
    colorMap["fire"] = qRgb(252, 130, 0);
    colorMap["wood"] = qRgb(166, 139, 102);
    colorMap["stone"] = qRgb(80, 80, 80);
    colorMap["metal"] = qRgb(179, 179, 179);

    for (int x = 0; x < 4096; ++x) {
        for (int y = 0; y < 4096; ++y) {
            QString material = tree->get(x, y);

            if (colorMap.contains(material))
                result.setPixel(x, y, colorMap[material]);
        }
    }

    return result;
}

QImage SectorConverter::createHeightImage() const
{
    QScopedPointer<CharQuadtree> tree(createHeightQuadtree(mZoneTemplate));

    QImage result(4096, 4096, QImage::Format_ARGB32);

    result.fill(0);

    for (int x = 0; x < 4096; ++x) {
        for (int y = 0; y < 4096; ++y) {
            char height = tree->get(x, y);

            if (height == 0)
                continue;
            else if (height < 0)
                result.setPixel(x, y, qRgb(- height * 2, 0, 0));
            else
                result.setPixel(x, y, qRgb(0, height * 2, 0));
        }
    }

    return result;
}

QImage SectorConverter::createVisionExtendImage() const
{
    QScopedPointer<BoolQuadtree> tree(createVisionExtendQuadtree(mZoneTemplate));

    QImage result(4096, 4096, QImage::Format_ARGB32);

    result.fill(0);

    for (int x = 0; x < 4096; ++x) {
        for (int y = 0; y < 4096; ++y) {
            bool enabled = tree->get(x, y);

            if (enabled)
                result.setPixel(x, y, qRgb(0, 255, 0));
        }
    }

    return result;
}

QImage SectorConverter::createVisionBaseImage() const
{
    QScopedPointer<BoolQuadtree> tree(createVisionBaseQuadtree(mZoneTemplate));

    QImage result(4096, 4096, QImage::Format_ARGB32);

    result.fill(0);

    for (int x = 0; x < 4096; ++x) {
        for (int y = 0; y < 4096; ++y) {
            bool enabled = tree->get(x, y);

            if (enabled)
                result.setPixel(x, y, qRgb(0, 255, 0));
        }
    }

    return result;
}

QImage SectorConverter::createVisionArchwayImage() const
{
    QScopedPointer<BoolQuadtree> tree(createVisionArchwayQuadtree(mZoneTemplate));

    QImage result(4096, 4096, QImage::Format_ARGB32);

    result.fill(0);

    for (int x = 0; x < 4096; ++x) {
        for (int y = 0; y < 4096; ++y) {
            bool enabled = tree->get(x, y);

            if (enabled)
                result.setPixel(x, y, qRgb(0, 255, 0));
        }
    }

    return result;
}

QImage SectorConverter::createVisionEndImage() const
{
    QScopedPointer<BoolQuadtree> tree(createVisionEndQuadtree(mZoneTemplate));

    QImage result(4096, 4096, QImage::Format_ARGB32);

    result.fill(0);

    for (int x = 0; x < 4096; ++x) {
        for (int y = 0; y < 4096; ++y) {
            bool enabled = tree->get(x, y);

            if (enabled)
                result.setPixel(x, y, qRgb(0, 255, 0));
        }
    }

    return result;
}

static BoolQuadtree *createQuadtreeWalkable(const ZoneTemplate *zoneTemplate) {
    // We'll use a grid of 4096x4096 tiles.
    BoolQuadtree *quadtree = new BoolQuadtree(4096, false);

    foreach (const TileSector &sector, zoneTemplate->tileSectors()) {
        int xoffset = sector.x * SectorSidelength;
        int yoffset = sector.y * SectorSidelength;

        for (int x = 0; x < SectorSidelength; ++x) {
            for (int y = 0; y < SectorSidelength; ++y) {
                int actualX = (xoffset + x) * 3;
                int actualY = (yoffset + y) * 3;

                uint bitfield = sector.tiles[x][y].bitfield;

                if (!(bitfield & (TILE_BLOCKS_UL|TILE_FLYOVER_UL)))
                    quadtree->set(actualX, actualY, true);
                if (!(bitfield & (TILE_BLOCKS_UM|TILE_FLYOVER_UM)))
                    quadtree->set(actualX + 1, actualY, true);
                if (!(bitfield & (TILE_BLOCKS_UR|TILE_FLYOVER_UR)))
                    quadtree->set(actualX + 2, actualY, true);

                if (!(bitfield & (TILE_BLOCKS_ML|TILE_FLYOVER_ML)))
                    quadtree->set(actualX, actualY + 1, true);
                if (!(bitfield & (TILE_BLOCKS_MM|TILE_FLYOVER_MM)))
                    quadtree->set(actualX + 1, actualY + 1, true);
                if (!(bitfield & (TILE_BLOCKS_MR|TILE_FLYOVER_MR)))
                    quadtree->set(actualX + 2, actualY + 1, true);

                if (!(bitfield & (TILE_BLOCKS_LL|TILE_FLYOVER_LL)))
                    quadtree->set(actualX, actualY + 2, true);
                if (!(bitfield & (TILE_BLOCKS_LM|TILE_FLYOVER_LM)))
                    quadtree->set(actualX + 1, actualY + 2, true);
                if (!(bitfield & (TILE_BLOCKS_LR|TILE_FLYOVER_LR)))
                    quadtree->set(actualX + 2, actualY + 2, true);
            }
        }
    }

    quadtree->compact();

    return quadtree;
}

static BoolQuadtree *createQuadtreeFlyable(const ZoneTemplate *zoneTemplate) {
    // We'll use a grid of 4096x4096 tiles.
    BoolQuadtree *quadtree = new BoolQuadtree(4096, false);

    foreach (const TileSector &sector, zoneTemplate->tileSectors()) {
        int xoffset = sector.x * SectorSidelength;
        int yoffset = sector.y * SectorSidelength;

        for (int x = 0; x < SectorSidelength; ++x) {
            for (int y = 0; y < SectorSidelength; ++y) {
                int actualX = (xoffset + x) * 3;
                int actualY = (yoffset + y) * 3;

                uint bitfield = sector.tiles[x][y].bitfield;

                if (!(bitfield & (TILE_BLOCKS_UL)))
                    quadtree->set(actualX, actualY, true);
                if (!(bitfield & (TILE_BLOCKS_UM)))
                    quadtree->set(actualX + 1, actualY, true);
                if (!(bitfield & (TILE_BLOCKS_UR)))
                    quadtree->set(actualX + 2, actualY, true);

                if (!(bitfield & (TILE_BLOCKS_ML)))
                    quadtree->set(actualX, actualY + 1, true);
                if (!(bitfield & (TILE_BLOCKS_MM)))
                    quadtree->set(actualX + 1, actualY + 1, true);
                if (!(bitfield & (TILE_BLOCKS_MR)))
                    quadtree->set(actualX + 2, actualY + 1, true);

                if (!(bitfield & (TILE_BLOCKS_LL)))
                    quadtree->set(actualX, actualY + 2, true);
                if (!(bitfield & (TILE_BLOCKS_LM)))
                    quadtree->set(actualX + 1, actualY + 2, true);
                if (!(bitfield & (TILE_BLOCKS_LR)))
                    quadtree->set(actualX + 2, actualY + 2, true);
            }
        }
    }

    quadtree->compact();

    return quadtree;
}

static CharQuadtree *createHeightQuadtree(const ZoneTemplate *zoneTemplate) {
    // We'll use a grid of 4096x4096 tiles.
    CharQuadtree *quadtree = new CharQuadtree(4096, 0);

    foreach (const TileSector &sector, zoneTemplate->tileSectors()) {
        if (!sector.hasNegativeHeight)
            continue;

        int xoffset = sector.x * SectorSidelength;
        int yoffset = sector.y * SectorSidelength;

        for (int x = 0; x < SectorSidelength * 3; ++x) {
            for (int y = 0; y < SectorSidelength * 3; ++y) {
                int actualX = xoffset * 3 + x;
                int actualY = yoffset * 3 + y;

                char height = sector.negativeHeight[x][y];
                if (height != 0)
                    quadtree->set(actualX, actualY, - height);
            }
        }
    }

    quadtree->compact();

    return quadtree;
}

template<uint visionFlag>
BoolQuadtree *createVisionFlagQuadtree(const ZoneTemplate *zoneTemplate) {
    // We'll use a grid of 4096x4096 tiles.
    BoolQuadtree *quadtree = new BoolQuadtree(4096, false);

    foreach (const TileSector &sector, zoneTemplate->tileSectors()) {
        int xoffset = sector.x * SectorSidelength;
        int yoffset = sector.y * SectorSidelength;

        for (int x = 0; x < SectorSidelength; ++x) {
            for (int y = 0; y < SectorSidelength; ++y) {

                for (int sx = 0; sx < 3; ++sx) {
                    for (int sy = 0; sy < 3; ++sy) {
                        int actualX = (xoffset + x) * 3 + sx;
                        int actualY = (yoffset + y) * 3 + sy;

                        bool enabled = sector.tiles[x][y].isVisionFlagSet(sx, sy, visionFlag);
                        if (enabled)
                            quadtree->set(actualX, actualY, true);
                    }
                }
            }
        }
    }

    quadtree->compact();

    return quadtree;
}

BoolQuadtree *createVisionExtendQuadtree(const ZoneTemplate *zoneTemplate) {
    return createVisionFlagQuadtree<1>(zoneTemplate);
}

BoolQuadtree *createVisionBaseQuadtree(const ZoneTemplate *zoneTemplate) {
    return createVisionFlagQuadtree<4>(zoneTemplate);
}

BoolQuadtree *createVisionArchwayQuadtree(const ZoneTemplate *zoneTemplate) {
    return createVisionFlagQuadtree<8>(zoneTemplate);
}

BoolQuadtree *createVisionEndQuadtree(const ZoneTemplate *zoneTemplate) {
    return createVisionFlagQuadtree<2>(zoneTemplate);
}

static StringQuadtree *createGroundMaterialQuadtree(const ZoneTemplate *zoneTemplate) {
    // We'll use a grid of 4096x4096 tiles.
    StringQuadtree *quadtree = new StringQuadtree(4096, QString::null);

    QHash<uint, QString> materials;
    materials[2] = "dirt";
    materials[3] = "grass";
    materials[4] = "water";
    materials[5] = "deepWater";
    materials[6] = "ice";
    materials[7] = "fire";
    materials[8] = "wood";
    materials[9] = "stone";
    materials[10] = "metal";
    materials[11] = "marsh";

    foreach (const TileSector &sector, zoneTemplate->tileSectors()) {
        int xoffset = sector.x * SectorSidelength;
        int yoffset = sector.y * SectorSidelength;

        for (int x = 0; x < SectorSidelength; ++x) {
            for (int y = 0; y < SectorSidelength; ++y) {
                QString material = materials.value(sector.tiles[x][y].footstepsSound, QString::null);

                if (material.isNull())
                    continue;

                for (int sx = 0; sx < 3; ++sx) {
                    for (int sy = 0; sy < 3; ++sy) {
                        int actualX = (xoffset + x) * 3 + sx;
                        int actualY = (yoffset + y) * 3 + sy;

                        quadtree->set(actualX, actualY, material);
                    }
                }
            }
        }
    }

    quadtree->compact();

    return quadtree;
}
