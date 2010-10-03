
#include "drawhelper.h"
#include "materialstate.h"

#include "fogofwar.h"
#include "pathfinder.h"

#include <QPointer>

namespace EvilTemple {

const uint Sidelength = 2880;
const uint SectorSidelength = 192;
const uint SectorsPerAxis = 15;
const float TileSidelength = 28.2842703f / 3.0f;

struct FogSectorBitmap {

    unsigned char bitfield[256*256];

    bool textureDirty;

    uint texture;

    void fogAll() {
        memset(bitfield, 0xFF, sizeof(bitfield));
        textureDirty = true;
    }

    void revealAll() {
        memset(bitfield, 0, sizeof(bitfield));
        textureDirty = true;
    }

    bool isRevealed(int x, int y) {
        return bitfield[y * 256 + x] == 0;
    }

    void unreveal(int x, int y) {
        if (!bitfield[y * 256 + x]) {
            bitfield[y * 256 + x] = 0xFF;
            textureDirty = true;
        }
    }

    void reveal(int x, int y) {
        if (bitfield[y * 256 + x]) {
            bitfield[y * 256 + x] = 0;
            textureDirty = true;
        }
    }

    void makeTexture() {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    void updateTexture() {
        // Upload texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                           bitfield);
        textureDirty = false;
    }


};

class FogOfWarData : public AlignedAllocation
{
public:
    FogOfWarData();

    QPointer<TileInfo> tileInfo;
    Pathfinder pathfinder;

    Box3d boundingBox;
    SharedMaterialState material;
    bool initialized;
    FogSectorBitmap bitmap[SectorsPerAxis][SectorsPerAxis];

    GLuint textureHandle;

    void initialize(RenderStates &renderStates);

    FogSectorBitmap *getSector(int x, int y);

    bool hasLineOfSight(const Vector4 &from, const Vector4 &to) const;

};

FogOfWarData::FogOfWarData() : initialized(false)
{
    glGenTextures(1, &textureHandle);

    for (int x = 0; x < SectorsPerAxis; x++) {
        for (int y = 0; y < SectorsPerAxis; y++) {
            bitmap[x][y].texture = 0;
            bitmap[x][y].textureDirty = true;
            bitmap[x][y].fogAll();
        }
    }
}

FogSectorBitmap *FogOfWarData::getSector(int x, int y)
{
    int sectorX = x / SectorSidelength;
    int sectorY = y / SectorSidelength;

    if (sectorX < 0
        || sectorY < 0
        || sectorX >= SectorsPerAxis
        || sectorY >= SectorsPerAxis)
        return NULL;

    return &bitmap[sectorX][sectorY];
}

void FogOfWarData::initialize(RenderStates &renderStates)
{
    if (initialized)
        return;

    initialized = true;
    material = SharedMaterialState::create();

    if (!material->createFromFile(":/material/fog_material.xml", renderStates)) {
        qFatal("Unable to load fog of war material: %s", qPrintable(material->error()));
    }
}

bool FogOfWarData::hasLineOfSight(const Vector4 &from, const Vector4 &to) const
{
    if (!tileInfo) {
        return false;
    }

    QPoint startTile(from.x() / TileInfo::UnitsPerTile, from.z() / TileInfo::UnitsPerTile);
    QPoint endTile(to.x() / TileInfo::UnitsPerTile, to.z() / TileInfo::UnitsPerTile);

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
            if (tileInfo->isVisionEnd(y, x))
                return false;
        } else {
            if (tileInfo->isVisionEnd(x, y))
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

FogOfWar::FogOfWar() : d(new FogOfWarData)
{
    setRenderCategory(Renderable::FogOfWar);

    Vector4 minCorner = Vector4(0, 0, 0, 1);
    Vector4 maxCorner = Vector4(Sidelength * TileSidelength, 0, Sidelength * TileSidelength, 1);
    d->boundingBox = Box3d(minCorner, maxCorner);
}

FogOfWar::~FogOfWar()
{
}

static QPoint vectorToTile(const Vector4 &center)
{
    int tileX = center.x() / TileSidelength;
    int tileY = center.x() / TileSidelength;
    return QPoint(tileX, tileY);
}

static Vector4 tileToVector(int x, int y)
{
    x *= TileSidelength;
    y *= TileSidelength;
    return Vector4(x, 0, y, 1);
}

void FogOfWar::revealAll()
{
    for (int x = 0; x < SectorsPerAxis; ++x) {
        for (int y = 0; y < SectorsPerAxis; ++y) {
            d->bitmap[x][y].revealAll();
        }
    }
}

void FogOfWar::reveal(const Vector4 &center, float radius)
{
    // Find center sector
    int centerX = center.x() / TileSidelength;
    int centerY = center.z() / TileSidelength;

    int radiusSquare = radius * radius;

    for (int tileX = centerX - radius; tileX <= centerX + radius; ++tileX) {
        int xdiff = (tileX - centerX);
        xdiff *= xdiff;

        for (int tileY = centerY - radius; tileY <= centerY + radius; ++tileY) {

            int ydiff = (tileY - centerY);
            ydiff *= ydiff;

            // Check that the tile is within the radius
            if (xdiff + ydiff > radiusSquare)
                continue;

            FogSectorBitmap *bitmap = d->getSector(tileX, tileY);

            if (bitmap) {
                int subtileX = tileX % SectorSidelength;
                int subtileY = tileY % SectorSidelength;

                // The LoS check is very expensive, skip it, if the tile is already revealed
                if (bitmap->isRevealed(subtileX, subtileY))
                    continue;

                // Check for line of sight from center (TODO: Naive approach)
                if (!d->hasLineOfSight(center, tileToVector(tileX, tileY)))
                    continue;

                bitmap->reveal(subtileX, subtileY);
            } else {
                qWarning("Position is not inside the map: %d,%d", tileX, tileY);
            }

        }
    }

}

struct FogOfWarDrawStrategy : public DrawStrategy {

    FogOfWarData *d;

    FogOfWarDrawStrategy(FogOfWarData *_d) : d(_d) {}

    void draw(const RenderStates &renderStates, MaterialPassState &state) const
    {
        int samplerLoc = state.program->uniformLocation("foggedSampler");
        int posAttr = state.program->attributeLocation("vertexPosition");
        int texAttr = state.program->attributeLocation("vertexTexCoord");

        for (int x = 0; x < SectorsPerAxis; ++x) {
            for (int y = 0; y < SectorsPerAxis; ++y) {
                FogSectorBitmap &bitmap = d->bitmap[x][y];

                glActiveTexture(GL_TEXTURE0);

                if (bitmap.texture == 0) {
                    bitmap.makeTexture();
                }

                glBindTexture(GL_TEXTURE_2D, bitmap.texture);

                if (bitmap.textureDirty)
                    bitmap.updateTexture();

                bindUniform<int>(samplerLoc, 0);

                float startX = x * SectorSidelength * TileSidelength;
                float startY = y * SectorSidelength * TileSidelength;
                float endX = (x + 1) * SectorSidelength * TileSidelength;
                float endY = (y + 1) * SectorSidelength * TileSidelength;

                glBegin(GL_QUADS);
                glVertexAttrib2f(texAttr, 0, 0);
                glVertexAttrib3f(posAttr, startX, 0, startY);
                glVertexAttrib2f(texAttr, 1, 0);
                glVertexAttrib3f(posAttr, endX, 0, startY);
                glVertexAttrib2f(texAttr, 1, 1);
                glVertexAttrib3f(posAttr, endX, 0, endY);
                glVertexAttrib2f(texAttr, 0, 1);
                glVertexAttrib3f(posAttr, startX, 0, endY);
                glEnd();
            }
        }
    }

};

void FogOfWar::render(RenderStates &renderStates, MaterialState *overrideMaterial)
{
    d->initialize(renderStates);

    FogOfWarDrawStrategy drawer(d.data());

    DrawHelper<FogOfWarDrawStrategy> drawHelper;
    drawHelper.draw(renderStates, d->material.data(), drawer, EmptyBufferSource());
}

const Box3d &FogOfWar::boundingBox()
{
    return d->boundingBox;
}

TileInfo *FogOfWar::tileInfo() const
{
    return d->tileInfo;
}

void FogOfWar::setTileInfo(TileInfo *tileInfo)
{
    d->tileInfo = tileInfo;
    d->pathfinder.setTileInfo(tileInfo);
}

}

