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

/*
    Base class for the game itself. It controls startup of the game
    and initialization of other classes.
 */
class GAME_EXPORT Game : public QObject
{
Q_OBJECT
Q_PROPERTY(QString dataPath READ dataPath WRITE setDataPath)
Q_PROPERTY(QString userDataPath READ userDataPath WRITE setUserDataPath)
public:
    explicit Game(QObject *parent = 0);
    ~Game();

    ScriptEngine *scriptEngine() const;

    /**
      Gets the data path used to load the game's static data files. When the game
      object is created, this path is automatically set to the data folder in the
      location of the executable file.
      */
    QString dataPath() const;

    /**
      Changes the data path used by the game. Calling this method only makes sense
      before the start method is called. The path will not be automatically created,
      since it is considered to be read-only (unless the game is in editor mode).
      */
    void setDataPath(const QString &dataPath);

    /**
      Gets the data path for per-user data. This is used for savegames, screenshots
      and user-generated characters. Upon startup a system-dependend path is chosen.

      On Windows this is relative to the My Documents folder, so it is writeable,
      while on Unix systems, a sub-directory of the home directory is chosen.
      */
    QString userDataPath() const;

    /**
      Changes the per-user data path. Calling this method only makes sense before
      the start method is called. The path will be created by the start method
      if it doesn't exist.
      */
    void setUserDataPath(const QString &userDataPath);

    /**
      Returns the collection of translated strings.
      */
    const Translations *translations() const;

public slots:
    bool start();

private:
    QScopedPointer<GameData> d;

    Q_DISABLE_COPY(Game)
};

}

Q_DECLARE_METATYPE(EvilTemple::Game*)

#endif // GAME_H
