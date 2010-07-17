#include <QStringList>

#include "qdirvfshandler.h"


QDirVfsHandler::QDirVfsHandler(const QDir &baseDir) : mBaseDir(baseDir)
{
}

QByteArray QDirVfsHandler::openFile(const QString &filename)
{
    QFile f(mBaseDir.absoluteFilePath(filename));

    if (f.open(QIODevice::ReadOnly)) {
        return f.readAll();
    }

    return QByteArray((char*)NULL, 0);
}

bool QDirVfsHandler::exists(const QString &filename)
{
    return mBaseDir.exists(filename);
}

static void addFiles(QDir dir, const QString &prefix, const QStringList &filters, QStringList &result)
{
    // Add all files in the current directory
    QStringList entries = dir.entryList(filters, QDir::Files);
    foreach (const QString &entry, entries) {
        result.append(prefix + entry);
    }

    entries = dir.entryList(filters, QDir::Dirs|QDir::NoDotAndDotDot|QDir::NoSymLinks);
    foreach (const QString &entry, entries) {
        QDir subdir = dir;
        subdir.cd(entry);
        addFiles(dir, prefix + entry + "/", filters, result);
    }
}

QStringList QDirVfsHandler::listFiles(const QString &path, const QString &filter)
{
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

    addFiles(mBaseDir, "", filters, result);

    return result;
}
