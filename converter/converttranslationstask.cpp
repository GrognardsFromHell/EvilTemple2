#include "converttranslationstask.h"
#include "virtualfilesystem.h"
#include "messagefile.h"

using namespace Troika;

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
        QString mesKey = QDir::toNativeSeparators(mesFile).replace(QDir::separator(), "/");
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

    writer->close();
}
