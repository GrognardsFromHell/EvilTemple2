
#include <QString>
#include <QDateTime>
#include <QDataStream>
#include <QImage>

#include "savegame.h"

namespace EvilTemple {

class SaveGameData
{
public:
    QString name;
    QDateTime created;
    QImage screenshot;
};

SaveGame::SaveGame(QObject *parent) :
    QObject(parent), d_ptr(new SaveGameData)
{
}

SaveGame::~SaveGame()
{
    qDebug("Deleting save game object.");
}

const QString &SaveGame::name() const
{
    return d_ptr->name;
}

const QDateTime &SaveGame::created() const
{
    return d_ptr->created;
}

const QImage &SaveGame::screenshot() const
{
    return d_ptr->screenshot;
}

QDataStream &operator<<(QDataStream &stream, const SaveGame &saveGame)
{
    SaveGameData *d_ptr = saveGame.d_ptr.data();
    return stream << d_ptr->created << d_ptr->name << d_ptr->screenshot;
}

QDataStream &operator>>(QDataStream &stream, SaveGame &saveGame)
{
    SaveGameData *d_ptr = saveGame.d_ptr.data();
    return stream >> d_ptr->created >> d_ptr->name >> d_ptr->screenshot;
}

}
