#ifndef NAVIGATIONMESHBUILDER_H
#define NAVIGATIONMESHBUILDER_H

#include <QList>
#include <QString>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class NavigationMesh;
class NavigationMeshBuilderData;

class NavigationMeshBuilder
{
public:
    NavigationMeshBuilder();
    ~NavigationMeshBuilder();

    bool build(const QString &filename, const QVector<Vector4> &startPositions);

    NavigationMesh *takeWalkableMesh() const;
    NavigationMesh *takeFlyableMesh() const;

    RegionLayers takeRegionLayers() const;
private:
    QScopedPointer<NavigationMeshBuilderData> d;
};

}

#endif // NAVIGATIONMESHBUILDER_H
