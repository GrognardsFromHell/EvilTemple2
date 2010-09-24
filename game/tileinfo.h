#ifndef TILEINFO_H
#define TILEINFO_H

#include <QObject>
#include <QMetaType>
#include <QPoint>

#include <common/quadtree.h>
#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class TileInfo : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit TileInfo(QObject *parent = 0);

    static const float PixelPerSubtile;

public slots:

    bool load(const QString &filename);

    bool isTileWalkable(int x, int y) const;
    bool isTileFlyable(int x, int y) const;
    const QString &tileMaterial(int x, int y) const;
    int tileHeight(int x, int y) const;
    bool isVisionEnd(int x, int y) const;

    bool isWalkable(const Vector4 &position) const;
    bool isFlyable(const Vector4 &position) const;
    const QString &material(const Vector4 &position) const;
    int height(const Vector4 &position) const;

private:
    BoolQuadtree mWalkableTiles;
    BoolQuadtree mFlyableTiles;
    StringQuadtree mTileMaterial;
    CharQuadtree mTileHeight;

    BoolQuadtree mVisionExtend;
    BoolQuadtree mVisionEnd;
    BoolQuadtree mVisionBase;
    BoolQuadtree mVisionArchway;

    static QPoint convertPosition(const Vector4 &position);
};

inline QPoint TileInfo::convertPosition(const Vector4 &position)
{
    return QPoint(position.x() / PixelPerSubtile, position.z() / PixelPerSubtile);
}

inline bool TileInfo::isWalkable(const Vector4 &position) const
{
    QPoint tilePos = convertPosition(position);
    return isTileWalkable(tilePos.x(), tilePos.y());
}

inline bool TileInfo::isFlyable(const Vector4 &position) const
{
    QPoint tilePos = convertPosition(position);
    return isTileFlyable(tilePos.x(), tilePos.y());
}

inline const QString &TileInfo::material(const Vector4 &position) const
{
    QPoint tilePos = convertPosition(position);
    return tileMaterial(tilePos.x(), tilePos.y());
}

inline int TileInfo::height(const Vector4 &position) const
{
    QPoint tilePos = convertPosition(position);
    return tileHeight(tilePos.x(), tilePos.y());
}

inline bool TileInfo::isTileWalkable(int x, int y) const
{
    return mWalkableTiles.get(x, y);
}

inline bool TileInfo::isTileFlyable(int x, int y) const
{
    return mFlyableTiles.get(x, y);
}

inline const QString &TileInfo::tileMaterial(int x, int y) const
{
    return mTileMaterial.get(x, y);
}

inline int TileInfo::tileHeight(int x, int y) const
{
    return mTileHeight.get(x, y);
}

inline bool TileInfo::isVisionEnd(int x, int y) const
{
    return mVisionEnd.get(x, y);
}

}

Q_DECLARE_METATYPE(EvilTemple::TileInfo*)

#endif // TILEINFO_H
