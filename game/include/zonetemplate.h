#ifndef ZONETEMPLATE_H
#define ZONETEMPLATE_H

#include <QObject>

namespace EvilTemple
{

    class ZoneTemplateData;
    class ZoneBackgroundMap;
    class GeometryMeshObject;

    class ZoneTemplate : public QObject
    {        
        Q_OBJECT

        friend class ZoneTemplateReader;
    public:
        explicit ZoneTemplate(quint32 id, QObject *parent = 0);
        ~ZoneTemplate();

        quint32 id() const;
        ZoneBackgroundMap *dayBackground() const;
        ZoneBackgroundMap *nightBackground() const;
        const QList<GeometryMeshObject*> &staticGeometry() const;
        const QList<GeometryMeshObject*> &clippingGeometry() const;

    protected:
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
        void addStaticGeometry(GeometryMeshObject *object);

        /**
          Adds a geometry object that is used to add depth information to the pre rendered background.
          @param object The geometry that is part of this zone template. The template takes ownership
                        of this pointer.
          */
        void addClippingGeometry(GeometryMeshObject *object);

    private:
        QScopedPointer<ZoneTemplateData> d_ptr;

        Q_DISABLE_COPY(ZoneTemplate);
    };

}

#endif // ZONETEMPLATE_H
