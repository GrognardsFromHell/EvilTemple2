
#include <QVariantMap>
#include <QSet>

#include <serializer.h>

#include <messagefile.h>
#include <virtualfilesystem.h>

#include "convertsoundstask.h"

ConvertSoundsTask::ConvertSoundsTask(IConversionService *service, QObject *parent)
    : ConversionTask(service, parent)
{
}

void ConvertSoundsTask::run()
{
    QScopedPointer<IFileWriter> writer(service()->createOutput("sounds"));

    QVariantMap soundMapping;

    QStringList soundIndexFiles = QStringList() << "sound/snd_critter.mes"
                                                << "sound/snd_interface.mes"
                                                << "sound/snd_misc.mes"
                                                << "sound/snd_item.mes"
                                                << "sound/snd_spells.mes";

    foreach (const QString &soundIndexFile, soundIndexFiles) {
        QHash<uint, QString> soundIndex = Troika::MessageFile::parse(service()->virtualFileSystem()->openFile(soundIndexFile));

        foreach (uint key, soundIndex.keys()) {
            QString filename = QDir::toNativeSeparators("sound/" + soundIndex[key].toLower());

            // Some entries are empty, others are directory names
            if (!filename.toLower().endsWith(".wav"))
                continue;

            soundMapping.insert(QString("%1").arg(key), QVariant(filename));
        }
    }

    // Copy all WAV/MP3 files
    QSet<QString> soundFiles;
    QSet<QString> mp3Files;
    foreach (const QString &filename, service()->virtualFileSystem()->listAllFiles("*.wav")) {
        soundFiles.insert(filename);
    }
    foreach (const QString &filename, service()->virtualFileSystem()->listAllFiles("*.mp3")) {
        mp3Files.insert(filename);
    }

    qDebug("Copying %d sound files.", soundFiles.size());

    uint workDone = 0;
    uint totalWork = soundFiles.size() + mp3Files.size();

    // Copy all sound files into the zip file
    foreach (const QString &filename, soundFiles) {
        assertNotAborted();

        writer->addFile(filename, service()->virtualFileSystem()->openFile(filename));

        if (++workDone % 10 == 0) {
            emit progress(workDone, totalWork);
        }
    }

    foreach (const QString &filename, mp3Files) {
        assertNotAborted();

        writer->addFile(filename, service()->virtualFileSystem()->openFile(filename), false);

        if (++workDone % 10 == 0) {
            emit progress(workDone, totalWork);
        }
    }

    // Create a mapping
    QJson::Serializer serializer;
    writer->addFile("sound/sounds.js", serializer.serialize(soundMapping));

    writer->close();
}

uint ConvertSoundsTask::cost() const
{
    return 10;
}

QString ConvertSoundsTask::description() const
{
    return "Converting sounds";
}
