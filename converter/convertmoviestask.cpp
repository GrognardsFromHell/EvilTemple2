

#include <QSet>
#include <QVariantMap>

#include <serializer.h>

#include <virtualfilesystem.h>

#include "convertmoviestask.h"

ConvertMoviesTask::ConvertMoviesTask(IConversionService *service, QObject *parent)
    : ConversionTask(service, parent)
{
}

void ConvertMoviesTask::run()
{
    QScopedPointer<IFileWriter> writer(service()->createOutput("movies"));

    QSet<QString> movieFiles;
    QHash<uint, QString> movies = service()->openMessageFile("movies/movies.mes");
    QVariantMap movieMapping;

    foreach (uint movieId, movies.keys()) {
        QStringList entryParts = movies[movieId].split(";");

        // Try a stupidity-fallback for the non-standard case
        if (entryParts.size() == 1)
            entryParts = movies[movieId].split(",");

        if (entryParts.size() != 3) {
            qDebug("Invalid movie entry: %s", qPrintable(movies[movieId]));
            continue;
        }

        QString movie = entryParts[0];
        QString music = entryParts[1];
        QString subtitles = entryParts[2];

        if (movie.endsWith(".bik", Qt::CaseInsensitive)) {
            QString filename = "movies/" + movie;

            if (!service()->virtualFileSystem()->exists(filename)) {
                qWarning("Skipping non-existant movie: %s", qPrintable(filename));
                continue;
            }

            movieFiles.insert(filename);

            QVariantMap movieEntry;
            movieEntry["type"] = "movie";
            movieEntry["filename"] = filename;
            if (!subtitles.isEmpty())
                movieEntry["subtitles"] = "mes/subtitles/" + subtitles;

            movieMapping[QString("%1").arg(movieId)] = movieEntry;
        } else if (movie.endsWith(".jpg", Qt::CaseInsensitive)) {
            QString filename = "slide/" + movie;

            if (!service()->virtualFileSystem()->exists(filename)) {
                qWarning("Skipping non-existant slide: %s", qPrintable(filename));
                continue;
            }

            movieFiles.insert(filename);

            QVariantMap movieEntry;
            movieEntry["type"] = "slide";
            movieEntry["filename"] = filename;
            movieEntry["music"] = music;
            if (!subtitles.isEmpty())
                movieEntry["subtitles"] = "mes/subtitles/" + subtitles;

            movieMapping[QString("%1").arg(movieId)] = movieEntry;
        } else {
            qWarning("Unknown movie type: %s", qPrintable(movie));
        }
    }

    qDebug("Copying %d movie files.", movieFiles.size());

    uint workDone = 0;
    uint totalWork = movieFiles.size();

    foreach (const QString &filename, movieFiles) {
        assertNotAborted();

        writer->addFile(filename, service()->virtualFileSystem()->openFile(filename), false);

        if (++workDone % 10 == 0) {
            emit progress(workDone, totalWork);
        }
    }

    // Create a mapping
    QJson::Serializer serializer;
    writer->addFile("movies/movies.js", serializer.serialize(movieMapping));

    writer->close();
}

uint ConvertMoviesTask::cost() const
{
    return 10;
}

QString ConvertMoviesTask::description() const
{
    return "Converting movies";
}
