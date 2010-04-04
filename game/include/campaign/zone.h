#ifndef ZONEMAP_H
#define ZONEMAP_H

#include <QObject>
#include <QSharedPointer>

#include "zonetemplate.h"

namespace EvilTemple
{

    class ZoneTemplate;
    class ZoneData;

    class Zone : public QObject
    {
    Q_OBJECT
    public:
        explicit Zone(const QSharedPointer<ZoneTemplate> &zoneTemplate, QObject *parent = 0);
        ~Zone();

        /**
          The maximum number of tiles on the X axis a map can have.
          */
        static const quint32 HorizontalTiles = 71;

        /**
          The maximum number of tiles on the Y axis a map can have.
          */
        static const quint32 VerticalTiles = 66;

        /**
          The side-length of a tile in pixels.
          */
        static const quint32 TileSideLength = 256;

    signals:

    public slots:

        const QSharedPointer<ZoneTemplate> &zoneTemplate() const;

    private:
        Q_DISABLE_COPY(Zone);
        QScopedPointer<ZoneData> d_ptr;

    };

}

#endif // ZONEMAP_H
