#include "savegames.h"

namespace EvilTemple {

class SaveGamesData
{
public:
    QString saveGamePath;
};

SaveGames::SaveGames(const QString &saveGamePath, QObject *parent) : d_ptr(new SaveGamesData)
{
    d_ptr->saveGamePath = saveGamePath;
}

SaveGames::~SaveGames()
{
}

/**
  Retrieves a list of all the save games in the save game folder.
  */
QList<SaveGame*> SaveGames::listSaves() const
{
    return QList<SaveGame*>();
}

}
