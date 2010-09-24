
#include <QStringList>

#include "conversion/qdirvfshandler.h"

QDirVfsHandler::QDirVfsHandler(const QDir &baseDir) : mBaseDir(baseDir)
{
}

QByteArray QDirVfsHandler::openFile(const QString &filename)
{
    if (filename.startsWith("movies") || filename.startsWith("sound")) {
        QFile f(mBaseDir.absoluteFilePath(filename));

        if (f.open(QIODevice::ReadOnly)) {
            return f.readAll();
        }
    }

    return QByteArray((char*)NULL, 0);
}

bool QDirVfsHandler::exists(const QString &filename)
{
    if (!filename.startsWith("movies") || filename.startsWith("sound"))
        return false;
    return mBaseDir.exists(filename);
}

static void addFiles(QDir dir, const QString &prefix, const QStringList &filters, QStringList &result)
{
    // Add all files in the current directory
    QStringList entries = dir.entryList(filters, QDir::Files);
    foreach (const QString &entry, entries) {
        result.append(prefix + entry);
    }

    entries = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot|QDir::NoSymLinks);
    foreach (const QString &entry, entries) {
        QDir subdir = dir;
        subdir.cd(entry);
        addFiles(subdir, prefix + entry + "/", filters, result);
    }
}

QStringList QDirVfsHandler::listFiles(const QString &path, const QString &filter)
{
    /*
     TODO: This fixes CO8 incompatibility for now by skipping everything other than
         movies and sound
    */
    if (!path.startsWith("music") || path.startsWith("sound"))
        return QStringList();

    QStringList result;

    QStringList filters;
    filters << filter;

    QDir dir = mBaseDir;
    if (!dir.cd(path))
        return result;

    QString prefix = path;
    if (!prefix.endsWith("/") && !prefix.endsWith("\\"))
        prefix.append("/");

    addFiles(dir, path, filters, result);

    return result;
}

QStringList QDirVfsHandler::listAllFiles(const QString &filenameFilter)
{
    QStringList result;

    QStringList filters;
    filters << filenameFilter;

    /*
     TODO: This fixes CO8 incompatibility for now by skipping everything other than
         movies and sound
    */
    QDir subdir = mBaseDir;
    if (subdir.cd("sound") && subdir.cd("music"))
        addFiles(subdir, "sound/music/", filters, result);
    subdir = mBaseDir;
    if (subdir.cd("movies"))
        addFiles(subdir, "movies/", filters, result);

    return result;
}
