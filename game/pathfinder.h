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

    /**
      Tries to find a path that moves the actor into range of a target.

      @param start The current actor position.
      @param target The position of the target.
      @param actorRadius Radius of the actor. Used to find traversable terrain and also taken into account for
                         moving into range.
      @param targetRadius Radius of the target. This has to include the actual reach of the actor and should be
                          slightly larger than the radius actually blocked by the target (if any), since a valid
                          goal will let the circle around the actor and the circle around the target touch.
                          This may lead to no path being found, if the target radius is exactly equal to the radius
                          blocked by the target.
      */
    QVector<Vector4> findPathIntoRange(const Vector4 &start,
                                       const Vector4 &target,
                                       float actorRadius,
                                       float targetRadius);

    /**
      Checks whether there is an uninterrupted line of sight between two points.
      */
    bool hasLineOfSight(const Vector4 &from, const Vector4 &to) const;

    /**
      Verifies a path for a given actor size.
      */
    bool isPathValid(const QVector<Vector4> &points, float actorRadius) const;

    /**
      Adds a dynamic obstacle that is respected during path calculations.
      If an obstacle with the same identifier already exists, the existing obstacle
      is modified with the given data.
      */
    void addObstacle(const QString &id, const Vector4 &position, float radius);

    /**
      Removes a dynamic obstacle.
      */
    void removeObstacle(const QString &id);

    /**
      Determines whether an actor can stand at a position on the world map.
      */
    bool canStandAt(const Vector4 &position, float actorRadius) const;

private:
    typedef QPair<Vector4, float> Obstacle;

    QPointer<TileInfo> mTileInfo;

    QHash<QString, Obstacle> mObstacles;

};

inline void Pathfinder::addObstacle(const QString &id, const Vector4 &position, float radius)
{
    mObstacles[id] = Obstacle(position, radius);
}

inline void Pathfinder::removeObstacle(const QString &id)
{
    mObstacles.remove(id);
}

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
