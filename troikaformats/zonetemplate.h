#ifndef ZONETEMPLATE_H
#define ZONETEMPLATE_H


#include <gamemath.h>
using namespace GameMath;

#include "troikaformatsglobal.h"

#include "objectfilereader.h"

#include <QObject>
#include <QQuaternion>
#include <QVector3D>


class QBox3D;

namespace Troika
{
    /**
      The side length of a square sector in tiles.
      */
    const static int SectorSidelength = 64;

    class ZoneTemplateData;
    class ZoneBackgroundMap;

    class TROIKAFORMATS_EXPORT GeometryObject {
    public:
        GeometryObject(const QVector3D &position, float rotation, const QString &mesh) :
                mPosition(position), mRotation(rotation), mScale(1, 1, 1), mMesh(mesh) {
        }

        GeometryObject(const QVector3D &position,
                       float rotation,
                       const QVector3D &scale,
                       const QString &mesh) :
                mPosition(position), mRotation(rotation), mScale(scale), mMesh(mesh) {}

        const QVector3D &position() const { return mPosition; }
        float rotation() const { return mRotation; }
        const QVector3D &scale() const { return mScale; }
        const QString &mesh() const { return mMesh; }
    private:

        QVector3D mPosition;
        float mRotation; // in degrees
        QVector3D mScale;
        QString mMesh;
    };

    struct Light
    {
        bool day;
        quint64 handle;
        quint32 type;
        quint8 r, g, b; // Diffuse/Specular
        quint8 unknown;
        quint8 ur, ug, ub, ua; // Unknown color
        Vector4 position;
        float dirX, dirY, dirZ;
        float range;
        float phi;
    };

    struct ParticleSystem
    {
        Light light;
        quint32 hash;
        quint32 id;
    };

    struct SectorTile
    {
        quint8 footstepsSound;
        quint8 unknown1[3];
        quint32 bitfield;
        quint64 unknown2;
    };

    class TileSector {
    public:
        uint x;
        uint y;
        SectorTile tiles[SectorSidelength][SectorSidelength];
    };

    /**
      A keyframe entry used for day/night transfer lighting.
      */
    struct LightKeyframe {
        uint hour;
        float red;
        float green;
        float blue;
    };

    class TROIKAFORMATS_EXPORT ZoneTemplate : public QObject
    {
        Q_OBJECT
    public:
        explicit ZoneTemplate(quint32 id, QObject *parent = 0);
        ~ZoneTemplate();

        quint32 id() const;
        ZoneBackgroundMap *dayBackground() const;
        ZoneBackgroundMap *nightBackground() const;
        const QList<GeometryObject*> &staticGeometry() const;
        const QList<GeometryObject*> &clippingGeometry() const;

        const QList<TileSector> &tileSectors() const;

        const QString &directory() const;

        const QPoint &startPosition() const; // Camera start position
        quint32 movie(); // Movie to play when entering
        bool isTutorialMap() const;
        bool isMenuMap() const; // This denotes the first map played in the background of the menu
        bool isUnfogged() const; // No fog of war
        bool isOutdoor() const;
        bool hasDayNightTransfer() const;
        bool allowsBedrest() const;

        const Light &globalLight() const;

        const QList<Light> &lights() const;

        const QList<ParticleSystem> &particleSystems() const;

        const QString &name() const; // Zone name (translated)

        /**
          Returns the visible box of the map. The user can only scroll within the bounds of this box
          on this map.
          */
        const QBox3D &scrollBox() const;

        const QList<GameObject*> &staticObjects() const;
        const QList<GameObject*> &mobiles() const;

        /**
          Sets the background to use during the day. If no day/night exchange is used
          for this zone, this background is also used during the night.
          */
        void setDayBackground(ZoneBackgroundMap *backgroundMap);

        /**
          Sets the background to use during the night. If no day/night exchange is used,
          this value is ignored.
          */
        void setNightBackground(ZoneBackgroundMap *backgroundMap);

        /**
          Adds a static geometry object. This could be a tree or a door for instance.
          @param object The geometry that is part of this zone template. The template takes ownership
                        of this pointer.
          */
        void addStaticGeometry(GeometryObject *object);

        /**
          Adds a geometry object that is used to add depth information to the pre rendered background.
          @param object The geometry that is part of this zone template. The template takes ownership
                        of this pointer.
          */
        void addClippingGeometry(GeometryObject *object);

        void addLight(const Light &light);
        void addParticleSystem(const ParticleSystem &particleSystem);

        void setName(const QString &name);
        void setDirectory(const QString &directory);
        void setStartPosition(const QPoint &startPosition);
        void setMovie(quint32 movie);
        void setTutorialMap(bool enabled);
        void setMenuMap(bool enabled);
        void setUnfogged(bool enabled);
        void setOutdoor(bool enabled);
        void setDayNightTransfer(bool enabled);
        void setBedrest(bool enabled);
        void setScrollBox(const QBox3D &scrollBox);
                void setGlobalLight(const Light &light);

        void addStaticObject(GameObject *gameObject);
        void addMobile(GameObject *gameObject);

        void addTileSector(const TileSector &tileSector);

        /**
          The following methods map the data from rules\daylight.mes
          */
        const QList<LightKeyframe> &lightingKeyframesDay2d() const;
        const QList<LightKeyframe> &lightingKeyframesDay3d() const;

        void setLightingKeyframesDay(const QList<LightKeyframe> &keyframes2d,
                                     const QList<LightKeyframe> &keyframes3d);

        const QList<LightKeyframe> &lightingKeyframesNight2d() const;
        const QList<LightKeyframe> &lightingKeyframesNight3d() const;

        void setLightingKeyframesNight(const QList<LightKeyframe> &keyframes2d,
                                     const QList<LightKeyframe> &keyframes3d);

    private:
        QScopedPointer<ZoneTemplateData> d_ptr;

        Q_DISABLE_COPY(ZoneTemplate);
    };

}

#endif // ZONETEMPLATE_H
