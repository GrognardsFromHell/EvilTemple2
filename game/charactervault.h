#ifndef CHARACTERVAULT_H
#define CHARACTERVAULT_H

#include <QMetaType>
#include <QObject>
#include <QDir>
#include <QScriptEngine>

namespace EvilTemple {

/**
  This object allows access to the repository of generated/predefined characters.
  */
class CharacterVault : public QObject
{
    Q_OBJECT
public:

    /**
      Constructs a character vault object.
      @param pregeneratedPath The path to the directory that contains pregenerated characters.
      @param userPath The path to the directory containing user generated characters.
      @param engine The script engine to use for creating objects.
      */
    explicit CharacterVault(const QString &pregeneratedPath,
                            const QString &userPath,
                            QScriptEngine *engine,
                            QObject *parent = 0);

public slots:

    /**
      Lists the characters contained in the vault.
      */
    QScriptValue list() const;

    /**
      Adds a new character to the vault and returns the corresponding filename.
      */
    QScriptValue add(const QVariantMap &character);

    /**
      Removes a character from the vault given its filename.
      Only user generated characters may be removed.
      */
    void remove(const QString &filename);

private:
    QDir mPregeneratedDirectory;
    QDir mUserDirectory;
    QScriptEngine *mEngine;
};

}

Q_DECLARE_METATYPE(EvilTemple::CharacterVault*)

#endif // CHARACTERVAULT_H
