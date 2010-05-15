#ifndef ZONETEMPLATE_H
#define ZONETEMPLATE_H

#include "troikaformatsglobal.h"

#include <QObject>
#include <QQuaternion>
#include <QVector3D>

class QBox3D;

namespace Troika
{
    class ZoneTemplateData;
    class ZoneBackgroundMap;

    class TROIKAFORMATS_EXPORT GeometryObject {
    public:
        GeometryObject(const QVector3D &position, const QQuaternion &rotation, const QString &mesh) :
                mPosition(position), mRotation(rotation), mScale(1, 1, 1), mMesh(mesh)
        , staticObject(false), customRotation(false), rotationFromPrototype(false) {
        }

        GeometryObject(const QVector3D &position,
                       const QQuaternion &rotation,
                       const QVector3D &scale,
                       const QString &mesh) :
                mPosition(position), mRotation(rotation), mScale(scale), mMesh(mesh) {}

        bool staticObject;
        bool rotationFromPrototype;
        bool customRotation;

        const QVector3D &position() const { return mPosition; }
        const QQuaternion &rotation() const { return mRotation; }
        const QVector3D &scale() const { return mScale; }
        const QString &mesh() const { return mMesh; }
    private:

        QVector3D mPosition;
        QQuaternion mRotation;
        QVector3D mScale;
        QString mMesh;
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

        const QString &directory() const;

        const QVector3D startPosition() const; // Camera start position
        quint32 movie(); // Movie to play when entering
        bool isTutorialMap() const;
        bool isMenuMap() const; // This denotes the first map played in the background of the menu
        bool isUnfogged() const; // No fog of war
        bool isOutdoor() const;
        bool hasDayNightTransfer() const;
        bool allowsBedrest() const;

        const QString &name() const; // Zone name (translated)

        /**
          Returns the visible box of the map. The user can only scroll within the bounds of this box
          on this map.
          */
        const QBox3D &scrollBox() const;

    public:
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

        void setName(const QString &name);
        void setDirectory(const QString &directory);
        void setStartPosition(const QVector3D &startPosition);
        void setMovie(quint32 movie);
        void setTutorialMap(bool enabled);
        void setMenuMap(bool enabled);
        void setUnfogged(bool enabled);
        void setOutdoor(bool enabled);
        void setDayNightTransfer(bool enabled);
        void setBedrest(bool enabled);
        void setScrollBox(const QBox3D &scrollBox);

    private:
        QScopedPointer<ZoneTemplateData> d_ptr;

        Q_DISABLE_COPY(ZoneTemplate);
    };

}

#endif // ZONETEMPLATE_H
