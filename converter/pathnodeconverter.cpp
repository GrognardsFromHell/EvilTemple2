
#include <QDataStream>
#include <QVector>

#include "constants.h"
#include "pathnodeconverter.h"

struct PathNode {
    uint id;
    float x;
    float y;
    QVector<uint> goals;
};

QDataStream &operator >>(QDataStream &stream, PathNode &node)
{
    // ToEE stores the location in an uint and an x/y offset
    uint x, y, goalCount;
    stream >> node.id >> x >> y >> node.x >> node.y >> goalCount;
    node.x = x * PixelPerWorldTile + node.x;
    node.y = y * PixelPerWorldTile + node.y;

    node.goals.resize(goalCount);

    for (uint i = 0; i < goalCount; ++i) {
        stream >> node.goals[i];
    }

    return stream;
}

class PathNodeConverterData
{
public:
    QString error;
    QVector<PathNode> nodes;

};

PathNodeConverter::PathNodeConverter() : d(new PathNodeConverterData)
{
}

PathNodeConverter::~PathNodeConverter()
{
}

bool PathNodeConverter::load(const QByteArray &data)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    if (stream.atEnd()) {
        d->error = "File is empty.";
        return false;
    }

    uint count;
    stream >> count;

    d->nodes.resize(count);

    for (uint i = 0; i < count; ++i) {
        stream >> d->nodes[i];
    }

    return true;
}

QVariantMap PathNodeConverter::convert()
{
    QVariantMap result;

    foreach (const PathNode &node, d->nodes) {
        QVariantMap nodeMap;
        nodeMap["x"] = node.x;
        nodeMap["y"] = node.y;
        QVariantList goals;
        foreach (uint goal, node.goals) {
            goals.append(QString("%1").arg(goal));
        }
        nodeMap["goals"] = goals;

        QString key = QString("%1").arg(node.id);
        result[key] = nodeMap;
    }

    return result;
}

const QString &PathNodeConverter::error() const
{
    return d->error;
}
