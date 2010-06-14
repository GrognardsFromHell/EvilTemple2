#ifndef SECTORMAP_H
#define SECTORMAP_H

#include <QObject>
#include <QPolygon>

#include "renderable.h"
#include "texture.h"

namespace EvilTemple {

class Scene;

class Sector : public Renderable
{
Q_OBJECT
public:
    Sector();

    void render(RenderStates &renderStates);

    const Box3d &boundingBox();

    void setTexture(const SharedTexture &texture);
    void addPolygon(const QRect &polygon);
private:
    SharedTexture mTexture;
    QList<QRect> mPolygons;
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

public slots:
    bool load(const QString &filename) const;

private:
    Scene *mScene;    
};

}

Q_DECLARE_METATYPE(EvilTemple::SectorMap*)

#endif // SECTORMAP_H
