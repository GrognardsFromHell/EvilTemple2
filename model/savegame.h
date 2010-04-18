#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "modelglobal.h"

#include <QObject>
#include <QScopedPointer>
#include <QDateTime>
#include <QString>
#include <QImage>
#include <QDataStream>

namespace EvilTemple
{

class SaveGameData;

class MODEL_EXPORT SaveGame : public QObject
{
Q_OBJECT

/*
    We have to declare the oprators as methods, but we still want to
    allow them to access the internal state of the savegame.
*/
friend QDataStream &operator<<(QDataStream &, const SaveGame &);
friend QDataStream &operator>>(QDataStream &, SaveGame &);

Q_PROPERTY(QDateTime created READ created)
Q_PROPERTY(QString name READ name)
Q_PROPERTY(QImage screenshot READ screenshot)

public:
    explicit SaveGame(QObject *parent = 0);
    ~SaveGame();

    const QDateTime &created() const;
    const QString &name() const;
    const QImage &screenshot() const;

public slots:

private:
    QScopedPointer<SaveGameData> d_ptr;

    Q_DISABLE_COPY(SaveGame)
};

QDataStream &operator<<(QDataStream &, const SaveGame &);
QDataStream &operator>>(QDataStream &, SaveGame &);

}

#endif // SAVEGAME_H
