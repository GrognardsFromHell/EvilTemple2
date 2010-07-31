
#include "drawhelper.h"
#include "materialstate.h"

#include "fogofwar.h"

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
    }

    bool isRevealed(int x, int y) {
        return bitfield[y * 256 + x];
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

    Box3d boundingBox;
    SharedMaterialState material;
    bool initialized;
    FogSectorBitmap bitmap[SectorsPerAxis][SectorsPerAxis];

    GLuint textureHandle;

    void initialize(RenderStates &renderStates);

    FogSectorBitmap *getSector(int x, int y);
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

            if (xdiff + ydiff > radiusSquare)
                continue;

            FogSectorBitmap *bitmap = d->getSector(tileX, tileY);

            if (bitmap) {
                int subtileX = tileX % SectorSidelength;
                int subtileY = tileY % SectorSidelength;

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

}

