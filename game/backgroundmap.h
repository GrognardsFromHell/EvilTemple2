#ifndef BACKGROUNDMAP_H
#define BACKGROUNDMAP_H

#include <QScopedPointer>

namespace EvilTemple {

class RenderStates;
class BackgroundMapData;

class BackgroundMap
{
public:
    BackgroundMap(const RenderStates &states);
    ~BackgroundMap();

    void render();

    bool setMapDirectory(const QString &directory);

private:
    QScopedPointer<BackgroundMapData> d;
    Q_DISABLE_COPY(BackgroundMap)
};

}

#endif // BACKGROUNDMAP_H
