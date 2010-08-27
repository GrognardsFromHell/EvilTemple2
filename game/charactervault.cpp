#include "charactervault.h"

#include <serializer.h>

namespace EvilTemple {

CharacterVault::CharacterVault(const QString &pregeneratedPath,
                               const QString &userPath,
                               QScriptEngine *engine,
                               QObject *parent)
                                   : mPregeneratedDirectory(pregeneratedPath),
                                   mUserDirectory(userPath),
                                   mEngine(engine),
                                   QObject(parent)
{
    QDir parentDir = mPregeneratedDirectory;
    parentDir.cdUp();
    if (!parentDir.exists(mPregeneratedDirectory.dirName())) {
        parentDir.mkdir(mPregeneratedDirectory.dirName());
    }

    parentDir = mUserDirectory;
    parentDir.cdUp();
    if (!parentDir.exists(mUserDirectory.dirName())) {
        parentDir.mkdir(mUserDirectory.dirName());
    }
}

QScriptValue CharacterVault::list() const
{
    return QScriptValue();
}

static const char *reservedCharacters = "<>:\"/\\|?*";

/**
  This method will try to produce a valid filename given a character name.
  */
QString mangleCharacterName(const QString &name)
{
    QString newName = name;

    for (size_t i = 0; i < strlen(reservedCharacters); ++i)
        newName.remove(reservedCharacters[i]);

    return newName;
}

QScriptValue CharacterVault::add(const QVariantMap &character)
{
    QString charname = character["name"].toString();

    QString basename = mangleCharacterName(charname);

    // Make attempts at writing the file while modifying the filename to compensate for existing files

    qDebug("Mangled charname %s to %s.", qPrintable(charname), qPrintable(basename));

    QString filename = basename + ".js";

    if (mUserDirectory.exists(filename)) {
        bool success = false;
        for (int i = 1; i < 100; ++i) {
            filename = QString("%1 (%2).js").arg(basename).arg(i);

            if (!mUserDirectory.exists(filename)) {
                success = true;
                break;
            }
        }

        if (!success) {
            return mEngine->currentContext()->throwError(QString("Unable to create file: %1").arg(filename));
        }
    }

    QFile output(mUserDirectory.filePath(filename));

    if (!output.open(QIODevice::WriteOnly)) {
        return mEngine->currentContext()->throwError(QString("Unable to create file: %1").arg(filename));
    }

    QJson::Serializer serializer;
    output.write(serializer.serialize(character));

    return QScriptValue(filename);
}

void CharacterVault::remove(const QString &filename)
{

}

}
