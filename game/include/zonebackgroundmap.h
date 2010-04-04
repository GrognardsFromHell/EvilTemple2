#ifndef ZONEBACKGROUNDMAP_H
#define ZONEBACKGROUNDMAP_H

#include <QObject>

class QGLContext;
class Game;

namespace EvilTemple
{
    class ZoneBackgroundMapData;

    class ZoneBackgroundMap : public QObject
    {
        Q_OBJECT
    public:
        explicit ZoneBackgroundMap(const QString &directory, QObject *parent = 0);
        ~ZoneBackgroundMap();

        void draw(const Game &game, QGLContext *context);

    private:
        QScopedPointer<ZoneBackgroundMapData> d_ptr;

        Q_DISABLE_COPY(ZoneBackgroundMap);
    };

}

#endif // ZONEBACKGROUNDMAP_H
