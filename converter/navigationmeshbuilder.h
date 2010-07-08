#ifndef NAVIGATIONMESHBUILDER_H
#define NAVIGATIONMESHBUILDER_H

#include <QList>
#include <QString>

#include "zonetemplate.h"

#include <gamemath.h>
using namespace GameMath;

class NavigationMeshBuilder
{
public:
    static QByteArray build(const Troika::ZoneTemplate *tpl, const QVector<QPoint> &startPositions);
};

#endif // NAVIGATIONMESHBUILDER_H
