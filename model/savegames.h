#ifndef SAVEGAMES_H
#define SAVEGAMES_H

#include "modelglobal.h"

#include <QObject>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QList>

#include "savegame.h"

namespace EvilTemple
{

class SaveGamesData;

/**
  Models the savegame system. Allows save games to be iterated without actually being loaded
  (used by the load/save game dialogs).
  */
class MODEL_EXPORT SaveGames : public QObject
{
Q_OBJECT
public:
    /**
      Constructs the save game registry with the given storage path for save games.
      */
    explicit SaveGames(const QString &saveGamePath, QObject *parent = 0);
    ~SaveGames();

signals:

public slots:

    /**
      Retrieves a list of all the save games in the save game folder.
      */
    QList<SaveGame*> listSaves() const;

private:
    QScopedPointer<SaveGamesData> d_ptr;

    Q_DISABLE_COPY(SaveGames);
};

}

#endif // SAVEGAMES_H
