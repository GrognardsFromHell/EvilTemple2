#ifndef SECTORMAP_H
#define SECTORMAP_H

#include <QObject>
#include <QPolygon>
#include <QScopedPointer>

#include "renderable.h"
#include "texture.h"

namespace EvilTemple {

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
private:
    SharedTexture mTexture;
    Box3d mBoundingBox;
};

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
    bool load(const QString &filename) const;

    QVector<Vector4> findPath(const Vector4 &start, const Vector4 &end);

private:
    QScopedPointer<SectorMapData> d;
};

}

Q_DECLARE_METATYPE(EvilTemple::SectorMap*)

#endif // SECTORMAP_H
