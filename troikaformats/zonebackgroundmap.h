#ifndef ZONEBACKGROUNDMAP_H
#define ZONEBACKGROUNDMAP_H

#include "troikaformatsglobal.h"

#include <QObject>

class QGLContext;
class Game;

namespace Troika
{
    class ZoneBackgroundMapData;

    class TROIKAFORMATS_EXPORT ZoneBackgroundMap : public QObject
    {
        Q_OBJECT
    public:
        explicit ZoneBackgroundMap(const QString &directory, QObject *parent = 0);
        ~ZoneBackgroundMap();

        const QString &directory() const;

    private:
        QScopedPointer<ZoneBackgroundMapData> d_ptr;

        Q_DISABLE_COPY(ZoneBackgroundMap);
    };

}

#endif // ZONEBACKGROUNDMAP_H
