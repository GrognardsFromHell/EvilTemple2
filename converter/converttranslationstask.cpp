
#include <QVariantMap>
#include <QList>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QRegExp>

#include <serializer.h>

#include "converttranslationstask.h"
#include "virtualfilesystem.h"
#include "messagefile.h"
#include "util.h"

using namespace Troika;

static void convertHelp(IConversionService *service, IFileWriter *writer)
{
    QByteArray helpFile = service->virtualFileSystem()->openFile("mes/help.tab");

    QVariantMap result;

    QList<QByteArray> lines = helpFile.split('\n');

    foreach (QByteArray line, lines) {
        if (line.trimmed().isEmpty())
            continue;

        // This is a state-machine style parser
        QStringList parts;
        bool inVerticalTab = false;
        int tokenStart = 0;
        int tokenLength = 0;

        for (int i = 0; parts.size() < 5 && i < line.length(); ++i) {
            if (line[i] == '\x0b') {
                inVerticalTab = true;
            } else if (line[i] == '\t') {
                parts.append(QString::fromLocal8Bit(line.mid(tokenStart, tokenLength)));
                tokenLength = 0;
                tokenStart = i + 1;
                inVerticalTab = false;
            } else {
                if (inVerticalTab) {
                    inVerticalTab = false;
                    parts.append(QString::fromLocal8Bit(line.mid(tokenStart, tokenLength)));
                    tokenLength = 0;
                    tokenStart = i;
                }

                tokenLength++;
            }
        }

        if (parts.size() != 5) {
            qWarning("Invalid line %s in help file.", line.constData());
            continue;
        }

        QString id = parts[0].trimmed();
        QString parentId = parts[1].trimmed();
        QStringList refFrom = parts[2].split(QRegExp("\\s+"), QString::SkipEmptyParts);
        QString unk = parts[3].trimmed();
        QString title = parts[4].trimmed();
        QString text = line.mid(tokenStart).trimmed();

        text.replace('\b', '\n'); // They used vertical tabs instead of newlines.

        QVariantMap entry;
        if (!parentId.isEmpty()) {
            entry["parent"] = parentId;
        }
        if (!refFrom.isEmpty()) {
            QVariantList refdFrom;
            foreach (QString ref, refFrom)
                refdFrom.append(ref.trimmed());
            entry["referencedBy"] = refdFrom;
        }
        if (!unk.isEmpty()) {
            entry["unk"] = unk;
        }
        entry["title"] = title;
        entry["text"] = text;
        result[id] = entry;
    }

    QJson::Serializer serializer;
    writer->addFile("help.js", serializer.serialize(result));
}

ConvertTranslationsTask::ConvertTranslationsTask(IConversionService *service, QObject *parent)
    : ConversionTask(service, parent)
{
}

uint ConvertTranslationsTask::cost() const
{
    return 2;
}

QString ConvertTranslationsTask::description() const
{
    return "Convert translations";
}

void ConvertTranslationsTask::run()
{
    QScopedPointer<IFileWriter> writer(service()->createOutput("translations"));

    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    QStringList mesFiles = service()->virtualFileSystem()->listAllFiles("*.mes");

    foreach (const QString &mesFile, mesFiles) {
        QString mesKey = normalizePath(mesFile);
        mesKey.replace(QRegExp("\\.mes$"), "").toLower();

        if (!mesKey.startsWith("mes/") && !mesKey.startsWith("oemes/")) {
            continue;
        }

        QHash<uint,QString> entries = MessageFile::parse(service()->virtualFileSystem()->openFile(mesFile));

        foreach (uint key, entries.keys()) {
            stream << QString("%1/%2").arg(mesKey).arg(key) << entries[key];
        }
    }

    writer->addFile("translation.dat", result);

    convertHelp(service(), writer.data());

    writer->close();
}
