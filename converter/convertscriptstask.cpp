
#include <QScriptEngine>
#include <QSet>

#include <serializer.h>

#include "exclusions.h"
#include "messagefile.h"
#include "virtualfilesystem.h"
#include "convertscriptstask.h"
#include "prototypeconverter.h"
#include "pythonconverter.h"
#include "util.h"

using namespace Troika;

ConvertScriptsTask::ConvertScriptsTask(IConversionService *service, QObject *parent)
    : ConversionTask(service, parent)
{
}

uint ConvertScriptsTask::cost() const
{
    return 5;
}

QString ConvertScriptsTask::description() const
{
    return "Converting scripts";
}

struct SoundSchemeEntry {
    enum Type {
        BackgroundMusic,
        CombatIntro,
        CombatMusic,
        Ambience
    };

    QString filename;
    Type type;
    uint volume;
    uint frequency;
    uint fromTime;
    uint toTime;
    bool scatter;
    uint volumeFrom;
    uint volumeTo;
    uint balanceFrom;
    uint balanceTo;
};

static bool parseSoundSchemeEntry(const QString &line, SoundSchemeEntry &entry)
{
    static QRegExp timePattern("/time:(\\d+)-(\\d+)");

    QStringList parts = line.split(' ');

    if (parts.isEmpty())
        return false;

    entry.filename = parts.takeFirst();
    entry.filename.replace('\\', '/');
    entry.filename.prepend("sound/");

    // Set defaults
    entry.volume = 100;
    entry.scatter = false;
    entry.fromTime = 0;
    entry.toTime = 23;
    entry.frequency = 50;
    entry.type = SoundSchemeEntry::Ambience;

    foreach (QString command, parts) {
        command = command.toLower();

        if (command == "/loop")
            entry.type = SoundSchemeEntry::BackgroundMusic;
        else if (command == "/combatintro")
            entry.type = SoundSchemeEntry::CombatIntro;
        else if (command == "/combatmusic")
            entry.type = SoundSchemeEntry::CombatMusic;
        else if (command.startsWith("/scatter"))
            entry.scatter = true;
        else if (command.startsWith("/freq:"))
            entry.frequency = command.mid(6).toUInt();
        else if (command.startsWith("/vol:")) {
            int index = command.indexOf('-');
            if (index == -1) {
                entry.volume = command.mid(5).toUInt();
            } else {
                entry.volumeFrom = command.mid(5, index - 5).toUInt();
                entry.volumeTo = command.mid(index + 1).toUInt();
            }
        } else if (command.startsWith("/bal:"))
            continue; // Ignored
        else if (timePattern.exactMatch(command)) {
            entry.fromTime = timePattern.cap(1).toUInt();
            entry.toTime = timePattern.cap(2).toUInt();
        } else {
            qWarning("Unknown sound scheme command: %s.", qPrintable(command));
            return false;
        }
    }

    return true;
}

static void convertSoundSchemes(IConversionService *service, IFileWriter *writer)
{
    QHash<uint, QString> soundSchemeIndex = service->openMessageFile("sound/schemeindex.mes");
    QHash<uint, QString> soundSchemeList = service->openMessageFile("sound/schemelist.mes");

    QVariantMap result;

    foreach (uint soundSchemeId, soundSchemeIndex.keys()) {
        if (soundSchemeId == 0)
            continue;

        QString name = soundSchemeIndex[soundSchemeId];

        QRegExp namePattern("(.*)\\s+#(\\d+)");
        if (!namePattern.exactMatch(name)) {
            qWarning("Unable to parse name of sound scheme: %s.", qPrintable(name));
            continue;
        }

        uint indexOffset = namePattern.cap(2).toUInt();

        QVariantMap scheme;
        scheme["name"] = namePattern.cap(1);
        QVariantList backgroundMusic;
        QVariantList ambientSounds;

        // Process 100 lines for the sound scheme.
        for (uint i = indexOffset; i < indexOffset + 100; ++i) {
            QString line = soundSchemeList.value(i, QString::null);

            if (line.isNull())
                continue;

            SoundSchemeEntry entry;
            if (!parseSoundSchemeEntry(line, entry)) {
                qWarning("Invalid sound scheme entry %d: %s.", i, qPrintable(line));
                continue;
            }

            QVariantMap newEntry;
            newEntry["filename"] = entry.filename;
            newEntry["volume"] = entry.volume;

            /**
              Presumably we could introduce a very generic script-guard system for ambience or
              background music sounds, instead of coding it like this.
              */
            if (entry.fromTime != 0 || entry.toTime != 23) {
                QVariantMap time;
                time["from"] = entry.fromTime;
                time["to"] = entry.toTime;
                newEntry["time"] = time;
            }

            switch (entry.type) {
            case SoundSchemeEntry::Ambience:
                if (entry.scatter) {
                    newEntry["scatter"] = true;
                }
                newEntry["frequency"] = entry.frequency;
                ambientSounds.append(newEntry);
                break;
            case SoundSchemeEntry::BackgroundMusic:
                backgroundMusic.append(newEntry);
                break;
            case SoundSchemeEntry::CombatIntro:
                scheme["combatIntro"] = newEntry;
                break;
            case SoundSchemeEntry::CombatMusic:
                scheme["combatMusic"] = newEntry;
                break;
            default:
                qWarning("Unknown type for scheme entry %d.", i);
                continue;
            }
        }

        scheme["ambientSounds"] = ambientSounds;
        scheme["backgroundMusic"] = backgroundMusic;

        result[QString("scheme-%1").arg(soundSchemeId)] = scheme;
    }

    QJson::Serializer serializer;

    writer->addFile("soundschemes.js", serializer.serialize(result));
}

static QVariantMap convertDialogScript(IConversionService *service, const QByteArray &rawScript, const QString &filename)
{
    PythonConverter converter(service);

    QVariantMap result;

    QString script = rawScript;

    QStringList lines = script.split("\n", QString::SkipEmptyParts);

    foreach (QString line, lines) {
        // Replace comment at end of line
        line.replace(QRegExp("//[^\\{\\}]*$"), "");
        line.replace(QRegExp("#[^\\{\\}]*$"), "");

        line = line.trimmed();

        QStringList parts = line.split(QRegExp("\\}\\s*\\{"), QString::KeepEmptyParts);
        for (int i = 0; i < parts.size(); ++i) {
            parts[i] = parts[i].trimmed(); // Trim spacing between parenthesis
        }

        if (parts.size() != 7)
            continue;

        parts[0] = parts[0].right(parts[0].length() - 1); // Skip opening bracket
        parts[parts.size() - 1] = parts[parts.size() - 1].left(parts[parts.size() - 1].length() - 1); // Skip closing bracket

        QString id = parts[0];
        QString text = parts[1];
        QString femaleText = parts[2];
        int intelligence = parts[3].toInt();
        QString guard = parts[4];
        QString nextId = parts[5];
        QString action = parts[6];

        // Fixes a broken line in the smithy's dialog
        if (guard.contains("game.areas[3] = 0"))
            continue;

        QVariantMap entry;
        entry["text"] = text;
        if (!femaleText.isEmpty() && femaleText != text)
            entry["femaleText"] = femaleText;
        if (intelligence != 0)
            entry["intelligence"] = intelligence;
        if (!guard.isEmpty()) {
            entry["guard"] = converter.convertDialogGuard(guard.toUtf8(), filename + ":" + id);
        }
        if (!nextId.isEmpty() && nextId != "0")
            entry["nextId"] = nextId.toUInt();
        if (!action.isEmpty())
            entry["action"] = converter.convertDialogAction(action.toUtf8(), filename + ":" + id);
        result[id] = entry;
    }

    return result;
}

static void convertLegacyMaps(IConversionService *service, IFileWriter *output)
{
    Exclusions exclusions;
    if (!exclusions.load(":/map_exclusions.txt")) {
        qWarning("Unable to load map exclusions in legacy mapping conversion.");
    }

    QHash<uint, QString> mapListMes = service->openMessageFile("rules/maplist.mes");

    QVariantMap legacyMaps;

    foreach (uint legacyId, mapListMes.keys()) {
        QString legacyIdString = QString("%1").arg(legacyId);

        // Only add it to the mapping if the map id wasn't excluded
        if (exclusions.isExcluded(legacyIdString))
            continue;

        QString entry = mapListMes[legacyId];
        QString mapId = entry.split(',')[0].toLower();
        legacyMaps[legacyIdString] = mapId;
    }

    QJson::Serializer serializer;
    output->addFile("legacy_maps.js", serializer.serialize(legacyMaps));
}

static void convertQuests(IConversionService *service, IFileWriter *output)
{
    QHash<uint, QString> questMes = service->openMessageFile("mes/gamequestlog.mes");

    QVariantMap quests;

    // At most 200 quests
    for (int i = 0; i < 200; ++i) {
        if (questMes.contains(i)) {
            QVariantMap quest;
            quest["name"] = questMes[i];
            quest["description"] = questMes[200 + i];
            quests[QString("quest-%1").arg(i)] = quest;
        }
    }

    QJson::Serializer serializer;
    output->addFile("quests.js", serializer.serialize(quests));
}

static void convertDialogScripts(IConversionService *service, IFileWriter *output)
{
    Troika::VirtualFileSystem *vfs = service->virtualFileSystem();

    QVariantMap dialogFiles;

    QJson::Serializer serializer;

    QSet<QString> filesWritten;

    QStringList allDialogFiles = vfs->listAllFiles("*.dlg");
    foreach (const QString &filename, allDialogFiles) {
        if (filesWritten.contains(filename))
            continue;

        if (!normalizePath(filename).startsWith("dlg/"))
            continue;

        QVariantMap dialogScript = convertDialogScript(service, vfs->openFile(filename), filename);

        if (dialogScript.isEmpty()) {
            qWarning("Dialog script %s is empty.", qPrintable(filename));
            continue;
        }

        filesWritten.insert(filename);

        QString shortFilename = filename.right(filename.length() - 4);
        uint id = shortFilename.left(5).toUInt();

        shortFilename.replace(".dlg", ".js");
        shortFilename.prepend("dialog/");

        dialogFiles[QString("%1").arg(id)] = shortFilename;
        output->addFile(shortFilename, serializer.serialize(dialogScript));
    }

    output->addFile("dialogs.js", serializer.serialize(dialogFiles));
}

static QByteArray convertScript(IConversionService *service, const QByteArray &pythonScript, const QString &filename)
{
    PythonConverter converter(service);

    return converter.convert(pythonScript, filename).toLocal8Bit();
}

static void convertScripts(IConversionService *service, IFileWriter *output)
{
    Troika::VirtualFileSystem *vfs = service->virtualFileSystem();

    QVariantMap scriptFiles;

    QSet<QString> filesWritten;

    QStringList allScriptFiles = vfs->listAllFiles("*.py");

    foreach (const QString &filename, allScriptFiles) {
        if (filesWritten.contains(filename))
            continue;

        if (!normalizePath(filename).startsWith("scr/spell")
            && !normalizePath(filename).startsWith("scr/py"))
            continue;

        QByteArray script = convertScript(service, vfs->openFile(filename), filename);

        if (script.isEmpty()) {
            qWarning("Sript %s is empty.", qPrintable(filename));
            continue;
        }

        filesWritten.insert(filename);

        QString shortFilename = filename.right(filename.length() - 4); // Skips scr/ at front of filename

        if (shortFilename.startsWith("py")) {
            uint id = shortFilename.left(7).right(5).toUInt();
            shortFilename.replace(".py", ".js");
            shortFilename.prepend("scripts/legacy/");
            scriptFiles[QString("%1").arg(id)] = shortFilename;
        } else if (shortFilename.startsWith("Spell")) {
            QString spellId = shortFilename.left(8).toLower();
            shortFilename.replace(".py", ".js");
            shortFilename.prepend("scripts/legacy/");
            scriptFiles[spellId] = shortFilename;
        } else {
            continue;
        }

        output->addFile(shortFilename, script);
    }

    // Process some scripts separately
    output->addFile("scripts/legacy/utilities.js", convertScript(service, vfs->openFile("scr/utilities.py"), "scr/utilities.py"));
    output->addFile("scripts/legacy/random_encounter.js", convertScript(service, vfs->openFile("scr/random_encounter.py"), "scr/random_encounter.py"));
    output->addFile("scripts/legacy/rumor_control.js", convertScript(service, vfs->openFile("scr/rumor_control.py"), "scr/rumor_control.py"));

    QJson::Serializer serializer;
    output->addFile("scripts.js", serializer.serialize(scriptFiles));
}

static QScriptValue readMesFile(QScriptContext *ctx, QScriptEngine *engine, Troika::VirtualFileSystem *vfs) {
    if (ctx->argumentCount() != 1) {
        return ctx->throwError("readMesFile takes one string argument.");
    }

    QString filename = ctx->argument(0).toString();

    QHash<uint,QString> mapping = MessageFile::parse(vfs->openFile(filename));

    QScriptValue result = engine->newObject();

    foreach (uint key, mapping.keys()) {
        result.setProperty(QString("%1").arg(key), QScriptValue(mapping[key]));
    }

    return result;
}

static QScriptValue addFile(QScriptContext *ctx, QScriptEngine *engine, IFileWriter *writer) {
    if (ctx->argumentCount() != 3) {
        return ctx->throwError("addFile takes three arguments: filename, content, compression.");
    }

    QString filename = ctx->argument(0).toString();
    QString content = ctx->argument(1).toString();
    int compression = ctx->argument(2).toInt32();

    writer->addFile(filename, content.toUtf8(), compression > 0);

    return QScriptValue(true);
}

static QScriptValue readTabFile(QScriptContext *ctx, QScriptEngine *engine, Troika::VirtualFileSystem *vfs) {
    if (ctx->argumentCount() != 1) {
        return ctx->throwError("readTabFile takes one string argument.");
    }

    QString filename = ctx->argument(0).toString();

    QByteArray content = vfs->openFile(filename);
    QList<QByteArray> lines = content.split('\n');
    QList< QList<QByteArray> > tabFileContent;

    for (int i = 0; i < lines.length(); ++i) {
        QByteArray line = lines[i];
        if (line.endsWith('\r')) {
            line = line.left(line.length() - 1);
        }

        if (line.isEmpty())
            continue;

        tabFileContent.append(line.split('\t'));
    }

    QScriptValue result = engine->newArray(tabFileContent.length());

    for (int i = 0; i < tabFileContent.length(); ++i) {
        QList<QByteArray> line = tabFileContent[i];
        QScriptValue record = engine->newArray(line.length());
        for (int j = 0; j < line.length(); ++j) {
            record.setProperty(j, QScriptValue(QString(line[j])));
        }
        result.setProperty(i, record);
    }

    return result;
}

void ConvertScriptsTask::convertPrototypes(IFileWriter *writer, QScriptEngine *engine)
{
    PrototypeConverter converter(service()->virtualFileSystem());

    QVariantMap result = converter.convertPrototypes(service()->prototypes());

    QScriptValue postprocess = engine->globalObject().property("postprocess");

    QScriptValueList args;
    args.append(engine->toScriptValue<QVariantMap>(result));

    postprocess.call(QScriptValue(), args);

    if (engine->hasUncaughtException()) {
        qWarning("JS Error: %s", qPrintable(engine->uncaughtException().toString()));
        return;
    }
}

void ConvertScriptsTask::run()
{
    QScopedPointer<IFileWriter> writer(service()->createOutput("scripts"));

    QScriptEngine engine;

    VirtualFileSystem *vfs = service()->virtualFileSystem();

    QScriptValue readMesFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)readMesFile, vfs);
    engine.globalObject().setProperty("readMes", readMesFn);

    QScriptValue readTabFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)readTabFile, vfs);
    engine.globalObject().setProperty("readTab", readTabFn);

    QScriptValue addFileFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)addFile, writer.data());
    engine.globalObject().setProperty("addFile", addFileFn);

    QFile scriptFile(":/scripts/converter.js");

    if (!scriptFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qFatal("Unable to open converter script: scripts/converter.js");
    }

    QString scriptCode = scriptFile.readAll();

    engine.evaluate(scriptCode, "scripts/converter.js");

    convertPrototypes(writer.data(), &engine);

    convertDialogScripts(service(), writer.data());

    convertQuests(service(), writer.data());

    convertScripts(service(), writer.data());

    convertLegacyMaps(service(), writer.data());

    convertSoundSchemes(service(), writer.data());

    writer->close();
}
