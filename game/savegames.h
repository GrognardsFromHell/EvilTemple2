#ifndef SAVEGAMES_H
#define SAVEGAMES_H

#include <QMetaType>
#include <QObject>
#include <QScriptEngine>
#include <QString>
#include <QDir>

namespace EvilTemple {

class SaveGames : public QObject
{
    Q_OBJECT
public:
    explicit SaveGames(const QString &savesPath, QScriptEngine *engine, QObject *parent = 0);

public slots:

    /**
      Creates a JS array that represents all save games. For each save game, a small JS object is returned
      that contains the following properties:
      - id (The save game's id)
      - name (The save game's name)
      - created (A JS Date object representing the time the savegame was created)
      - screenshot (URL pointing to the screenshot)
      */
    QScriptValue listSaves();

    /**
      Creates a new save game.

      @param name The user defined name of the new savegame.
      @param screenshot A URL pointing to a screenshot that will be copied to the savegame.
      @param payload The payload of the savegame.
      @returns The record (as per @ref listSaves) of the new save game.
      */
    QScriptValue save(const QString &name,
                      const QUrl &screenshot,
                      const QString &payload);

    /**
      Overwrites or saves a new savegame with a predefined id.

      This can be used to overwrite existing save-games, or create savegames in fixed slots, like
      the quicksaves.

      @param id The unique identifier of the savegame that should be overwritten or created. If this is null,
                a new unique identifier will be generated.
      @param name The user defined name of the savegame.
      @param screenshot A URL pointing to a screenshot that will be copied to the savegame.
      @param payload The payload of the savegame.
      @returns The record (as per @ref listSaves) of the new save game.
      */
    QScriptValue save(const QString &id,
                      const QString &name,
                      const QUrl &screenshot,
                      const QString &payload);

    /**
      Loads a savegame.

      @param id The savegame's unique id.
      @return The savegame's payload as a string.
      */
    QScriptValue load(const QString &id);

private:
    QDir mSavesDirectory;
    QScriptEngine *mEngine;
};

}

Q_DECLARE_METATYPE(EvilTemple::SaveGames*)

#endif // SAVEGAMES_H
