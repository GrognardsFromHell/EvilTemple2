#ifndef GAME_H
#define GAME_H

#include "gameglobal.h"

#include <QObject>
#include <QString>
#include <QMetaType>

namespace EvilTemple {

class GameData;
class Camera;
class ScriptEngine;
class Translations;
class Paths;

/*
    Base class for the game itself. It controls startup of the game
    and initialization of other classes.
 */
class GAME_EXPORT Game : public QObject
{
Q_OBJECT
public:
    explicit Game(const Paths *paths, QObject *parent = 0);
    ~Game();

    ScriptEngine *scriptEngine() const;

    /**
      Returns the collection of translated strings.
      */
    const Translations *translations() const;

    /**
      Returns the object describing the file system paths used by the game.
      */
    const Paths *paths() const;

public slots:
    bool start();

private:
    QScopedPointer<GameData> d;

    Q_DISABLE_COPY(Game)
};

}

Q_DECLARE_METATYPE(EvilTemple::Game*)

#endif // GAME_H
