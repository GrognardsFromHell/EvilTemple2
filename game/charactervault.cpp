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
    qDebug("User created character path: %s", qPrintable(userPath));
    qDebug("Pregenerated character path: %s", qPrintable(pregeneratedPath));

    if (!mUserDirectory.exists()) {
        qDebug("Creating user character directory: %s", qPrintable(mUserDirectory.absolutePath()));
        if (!mUserDirectory.root().mkpath(mUserDirectory.absolutePath())) {
            qWarning("Unable to create user directory for characters!");
        }
    }
}

QScriptValue CharacterVault::list() const
{
    QScriptValue result = mEngine->newArray();
    int i = 0;

    QStringList files = mPregeneratedDirectory.entryList(QStringList() << "*.js", QDir::Files);
    foreach (const QString &filename, files) {
        QFile file(mPregeneratedDirectory.absoluteFilePath(filename));
        if (file.open(QIODevice::ReadOnly)) {
            QScriptValue pregenChar = mEngine->newObject();
            pregenChar.setProperty("script", QString::fromUtf8(file.readAll()));
            result.setProperty(i++, pregenChar);
        } else {
            qWarning("Unable to open pregen character file: %s", qPrintable(filename));
        }
    }

    files = mUserDirectory.entryList(QStringList() << "*.js", QDir::Files);
    foreach (const QString &filename, files) {
        QFile file(mUserDirectory.absoluteFilePath(filename));
        if (file.open(QIODevice::ReadOnly)) {
            QScriptValue userChar = mEngine->newObject();
            userChar.setProperty("filename", filename);
            userChar.setProperty("script", QString::fromUtf8(file.readAll()));
            result.setProperty(i++, userChar);
        } else {
            qWarning("Unable to open user character file: %s", qPrintable(filename));
        }
    }

    return result;
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
            return mEngine->currentContext()->throwError(QString("Unable to find free filename for: %1").arg(filename));
        }
    }

    QFile output(mUserDirectory.absoluteFilePath(filename));

    if (!output.open(QIODevice::WriteOnly)) {
        return mEngine->currentContext()->throwError(QString("Unable to create file: %1").arg(output.fileName()));
    }

    QJson::Serializer serializer;
    output.write(serializer.serialize(character));

    return QScriptValue(filename);
}

void CharacterVault::remove(const QString &filename)
{
    // TODO: Ensure that filename is an element of the directory.
    mUserDirectory.remove(filename);
}

}
