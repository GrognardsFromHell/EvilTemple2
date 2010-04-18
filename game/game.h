#ifndef GAME_H
#define GAME_H

#include "gameglobal.h"

#include <QObject>

namespace EvilTemple {

class GameData;
class Camera;
class ScriptEngine;

/*
    Base class for the game itself. It controls startup of the game
    and initialization of other classes.
 */
class GAME_EXPORT Game : public QObject
{
Q_OBJECT
public:
    explicit Game(QObject *parent = 0);
    ~Game();

    Camera *camera() const;
    ScriptEngine *scriptEngine() const;

signals:

public slots:
    bool start();

private:
    QScopedPointer<GameData> d_ptr;

    Q_DISABLE_COPY(Game)
};

}

Q_DECLARE_METATYPE(EvilTemple::Game*)

#endif // GAME_H
