#ifndef BACKGROUNDMAP_H
#define BACKGROUNDMAP_H

#include <QObject>
#include <QScopedPointer>

namespace EvilTemple {

class RenderStates;
class BackgroundMapData;

class BackgroundMap : public QObject
{
Q_OBJECT
Q_PROPERTY(QString directory READ directory WRITE setDirectory)
public:
    BackgroundMap(const RenderStates &states);
    ~BackgroundMap();

    void render();

    bool setDirectory(const QString &directory);
    const QString &directory();

private:
    QScopedPointer<BackgroundMapData> d;
    Q_DISABLE_COPY(BackgroundMap)
};

}

Q_DECLARE_METATYPE(EvilTemple::BackgroundMap*)

#endif // BACKGROUNDMAP_H
