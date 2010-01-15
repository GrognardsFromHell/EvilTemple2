#ifndef GAME_H
#define GAME_H

#include <QObject>

namespace EvilTemple {

class GameData;
class VirtualFileSystem;
class Cursors;
class Materials;
class Camera;
class ZoneTemplates;
class Campaign;
class Prototypes;
class Models;
class ScriptEngine;

/*
    Base class for the game itself. It controls startup of the game
    and initialization of other classes.
 */
class Game : public QObject
{
Q_OBJECT
public:
    explicit Game(QObject *parent = 0);
    ~Game();

    VirtualFileSystem *virtualFileSystem() const;
    Cursors *cursors() const;
    Materials *materials() const;
    Camera *camera() const;
    ZoneTemplates *zoneTemplates() const;
    Campaign *campaign() const;
    Prototypes *prototypes() const;
    Models *models() const;
    ScriptEngine *scriptEngine() const;

signals:

public slots:
    bool start();

    void newCampaign();

private:
    QScopedPointer<GameData> d_ptr;
};

}

#endif // GAME_H
