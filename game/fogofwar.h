#ifndef FOGOFWAR_H
#define FOGOFWAR_H

#include <QScopedPointer>

#include "renderable.h"

namespace EvilTemple {

class FogOfWarData;

class FogOfWar : public Renderable
{
    Q_OBJECT
public:

    FogOfWar();
    ~FogOfWar();

    void render(RenderStates &renderStates, MaterialState *overrideMaterial);

    const Box3d &boundingBox();

signals:

public slots:

    void reveal(const Vector4 &center, float radius);

private:
    QScopedPointer<FogOfWarData> d;

};

}

#endif // FOGOFWAR_H
