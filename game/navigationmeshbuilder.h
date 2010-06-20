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

    NavigationMesh *build(const QString &filename, const QVector<Vector4> &startPositions);
private:
    QScopedPointer<NavigationMeshBuilderData> d;
};

}

#endif // NAVIGATIONMESHBUILDER_H
