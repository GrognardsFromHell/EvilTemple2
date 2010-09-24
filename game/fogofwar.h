#ifndef FOGOFWAR_H
#define FOGOFWAR_H

#include <QScopedPointer>

#include "renderable.h"
#include "sectormap.h"
#include "tileinfo.h"

namespace EvilTemple {

class FogOfWarData;

class FogOfWar : public Renderable
{
    Q_OBJECT
    Q_PROPERTY(EvilTemple::TileInfo *tileInfo READ tileInfo WRITE setTileInfo)
public:

    FogOfWar();
    ~FogOfWar();

    void render(RenderStates &renderStates, MaterialState *overrideMaterial);

    const Box3d &boundingBox();

    TileInfo *tileInfo() const;
    void setTileInfo(TileInfo *tileinfo);

signals:

public slots:

    void reveal(const Vector4 &center, float radius);

    void revealAll();

private:
    QScopedPointer<FogOfWarData> d;

};

}

#endif // FOGOFWAR_H
