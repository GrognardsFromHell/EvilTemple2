#ifndef SECTORMAP_H
#define SECTORMAP_H

#include <QObject>
#include <QScopedPointer>
#include <QPolygon>

#include "renderable.h"
#include "texture.h"

namespace EvilTemple {

struct NavMeshRect;

class Scene;

class SectorMapData;

class Sector : public Renderable
{
Q_OBJECT
public:
    Sector();

    void render(RenderStates &renderStates);

    const Box3d &boundingBox();

    void setTexture(const SharedTexture &texture);
    void addNavMeshRect(const NavMeshRect *polygon);
    void clearNavMeshRects();
private:
    SharedTexture mTexture;
    QVector<const NavMeshRect*> mNavMeshRects;
    Box3d mBoundingBox;
};

inline void Sector::clearNavMeshRects()
{
    mNavMeshRects.clear();
}

inline void Sector::setTexture(const SharedTexture &texture)
{
    mTexture = texture;
}

class SectorMap : public QObject
{
Q_OBJECT
public:
    SectorMap(Scene *scene);
    ~SectorMap();

public slots:
    bool load(const Vector4 &startPosition, const QString &filename) const;

    QVector<Vector4> findPath(const Vector4 &start, const Vector4 &end) const;

    bool hasLineOfSight(const Vector4 &from, const Vector4 &to) const;

private:
    QScopedPointer<SectorMapData> d;
};

}

Q_DECLARE_METATYPE(EvilTemple::SectorMap*)

#endif // SECTORMAP_H
