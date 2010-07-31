
#include <QUrl>
#include <QDateTime>

#include <QVariantMap>
#include <QVariantList>
#include <QVariant>

#include "savegames.h"

namespace EvilTemple {

struct IndexRecord {
    QString id;
    QString name;
    QDateTime created;
    QString screenshot;
};

static bool readIndexEntry(const QString &path, IndexRecord *record)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDataStream stream(&file);
    stream >> record->id >> record->name >> record->created >> record->screenshot;

    return stream.status() == QDataStream::Ok;
}

static bool writeIndexEntry(const QString &path, IndexRecord *record)
{
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        return false;
    }

    QDataStream stream(&file);
    stream << record->id << record->name << record->created << record->screenshot;

    file.close();

    return stream.status() == QDataStream::Ok;
}

static bool writePayload(const QString &path, const QString &payload)
{
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        return false;
    }

    file.write(payload.toUtf8());

    file.close();

    return true;
}

static QScriptValue loadPayload(const QString &path, QScriptEngine *engine)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        return engine->currentContext()->throwError("Couldn't open savegame payload: "  + path);
    }

    return QScriptValue(QString::fromUtf8(file.readAll(), file.size()));
}

static QScriptValue indexRecordToObject(QScriptEngine *engine, const IndexRecord &record)
{
    QScriptValue indexObject = engine->newObject();

    indexObject.setProperty("id", record.id);
    indexObject.setProperty("name", record.name);
    indexObject.setProperty("created", engine->newDate(record.created));

    QUrl screenshotUrl = QUrl::fromLocalFile(record.screenshot);
    indexObject.setProperty("screenshot", engine->newVariant(screenshotUrl));

    return indexObject;
}

SaveGames::SaveGames(const QString &savesPath, QScriptEngine *engine, QObject *parent) :
    QObject(parent), mSavesDirectory(savesPath), mEngine(engine)
{
    if (!mSavesDirectory.exists()) {
        qDebug("Creating saves directory at %s.", qPrintable(savesPath));
        if (!mSavesDirectory.mkpath(".")) {
            qWarning("Unable to create savegame directory.");
        }
    }
}

QScriptValue SaveGames::listSaves()
{
    QStringList directories = mSavesDirectory.entryList(QDir::Dirs|QDir::NoDotAndDotDot);

    QList<IndexRecord> indexRecords;
    indexRecords.reserve(directories.size());

    foreach (const QString &id, directories) {
        QDir saveDir(mSavesDirectory);
        if (!saveDir.cd(id)) {
            qWarning("Unable to cd into save directory: %s.", qPrintable(id));
            continue;
        }

        // Load the index.js file
        QString indexDatPath = saveDir.absoluteFilePath("index.dat");

        IndexRecord indexRecord;

        if (!readIndexEntry(indexDatPath, &indexRecord)) {
            qWarning("Unable to read index record file %s.", qPrintable(indexDatPath));
            continue;
        }

        indexRecord.screenshot = saveDir.absoluteFilePath(indexRecord.screenshot);

        indexRecords.append(indexRecord);
    }

    // Generate a script-value representation of the index
    QScriptValue result = mEngine->newArray(indexRecords.size());

    for (int i = 0; i < indexRecords.size(); ++i) {
        const IndexRecord &indexRecord = indexRecords.at(i);

        result.setProperty(i, indexRecordToObject(mEngine, indexRecord));
    }

    return result;
}

QScriptValue SaveGames::save(const QString &name,
                             const QUrl &screenshot,
                             const QString &payload)
{
    return save(QString::null, name, screenshot, payload);
}

QScriptValue SaveGames::save(const QString &id,
                             const QString &name,
                             const QUrl &screenshot,
                             const QString &payload)
{
    IndexRecord record;

    // Find a suitable id. TODO: This is not a very convincing algorithm.
    if (id.isNull()) {
        uint i = 1;
        do {
            record.id = QString("save%1").arg(i++);
        } while (mSavesDirectory.exists(record.id));
    } else {
        record.id = id;
    }

    // Create the directory if necessary.
    if (!mSavesDirectory.exists(record.id) && !mSavesDirectory.mkdir(record.id)) {
        QString errorMessage("Unable to create save directory: ");
        errorMessage.append(mSavesDirectory.absoluteFilePath(record.id));
        return mEngine->currentContext()->throwError(errorMessage);
    }

    QDir saveDirectory(mSavesDirectory);
    if (!saveDirectory.cd(record.id)) {
        mSavesDirectory.remove(record.id);
        QString errorMessage("Unable to change into save directory: ");
        errorMessage.append(saveDirectory.absoluteFilePath(record.id));
        return mEngine->currentContext()->throwError(errorMessage);
    }

    record.name = name;
    record.created = QDateTime::currentDateTimeUtc();
    record.screenshot = "screenshot.jpg";

    QString screenshotPath = saveDirectory.absoluteFilePath(record.screenshot);

    // Remove the screenshot if it already exists
    if (saveDirectory.exists(record.screenshot))
        saveDirectory.remove(record.screenshot);

    // Copy the screenshot from the provided location to the save directory.
    if (!QFile::copy(screenshot.toLocalFile(), screenshotPath)) {
        QFile::remove(screenshotPath);
        mSavesDirectory.remove(record.id);

        qWarning("Unable to copy screenshot from %s to %s.",
                 qPrintable(screenshot.toLocalFile()),
                 qPrintable(screenshotPath));
    }

    // Save our meta-data information to the save directory
    QString indexDatPath = saveDirectory.absoluteFilePath("index.dat");
    if (!writeIndexEntry(indexDatPath, &record)) {
        QFile::remove(indexDatPath);
        QFile::remove(screenshotPath);
        mSavesDirectory.remove(record.id);

        QString errorMessage("Unable to write index entry for savegame: " + indexDatPath);
        return mEngine->currentContext()->throwError(errorMessage);
    }

    // Finally store the actual payload of the savegame
    QString payloadPath = saveDirectory.absoluteFilePath("save.js");
    if (!writePayload(payloadPath, payload)) {
        QFile::remove(indexDatPath);
        QFile::remove(payloadPath);
        QFile::remove(screenshotPath);
        mSavesDirectory.remove(record.id);

        QString errorMessage("Unable to write savegame payload: " + payloadPath);
        return mEngine->currentContext()->throwError(errorMessage);
    }

    qDebug("Created new save @ %s.", qPrintable(mSavesDirectory.absoluteFilePath(record.id)));

    return indexRecordToObject(mEngine, record);
}

QScriptValue SaveGames::load(const QString &id)
{
    QDir saveDir(mSavesDirectory);

    if (!saveDir.cd(id)) {
        QString errorMessage = "Save directory does not exist: " + mSavesDirectory.absoluteFilePath(id);
        return mEngine->currentContext()->throwError(errorMessage);
    }

    QString payloadPath = saveDir.absoluteFilePath("save.js");

    return loadPayload(payloadPath, mEngine);
}

}
