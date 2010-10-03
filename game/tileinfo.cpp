
#include "tileinfo.h"

#include <QFile>
#include <QDataStream>

namespace EvilTemple {

TileInfo::TileInfo(QObject *parent) :
    QObject(parent)
{
}

bool TileInfo::load(const QString &filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream >> mWalkableTiles >> mFlyableTiles
            >> mTileMaterial >> mTileHeight >> mVisionExtend
            >> mVisionEnd >> mVisionBase >> mVisionArchway;

    return stream.status() == QDataStream::Ok;
}

const float TileInfo::UnitsPerTile = 28.2842703f / 3;

}
