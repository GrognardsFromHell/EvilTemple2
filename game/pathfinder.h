#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <QObject>
#include <QMetaType>
#include <QPointer>
#include <QVector>

#include <gamemath.h>
using namespace GameMath;

#include "tileinfo.h"

namespace EvilTemple {

class Pathfinder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(EvilTemple::TileInfo *tileInfo READ tileInfo WRITE setTileInfo)
public:
    Q_INVOKABLE explicit Pathfinder(QObject *parent = 0);

    void setTileInfo(TileInfo *tileInfo);
    TileInfo *tileInfo() const;

public slots:

    QVector<Vector4> findPath(const Vector4 &start, const Vector4 &end, float actorRadius) const;

    bool hasLineOfSight(const Vector4 &from, const Vector4 &to) const;

private:
    QPointer<TileInfo> mTileInfo;

};

inline void Pathfinder::setTileInfo(TileInfo *tileInfo)
{
    mTileInfo = tileInfo;
}

inline TileInfo *Pathfinder::tileInfo() const
{
    return mTileInfo;
}

}

Q_DECLARE_METATYPE(EvilTemple::Pathfinder*)

#endif // PATHFINDER_H
