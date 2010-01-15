#ifndef TROIKAARCHIVE_H
#define TROIKAARCHIVE_H

#include <QObject>
#include <QVector>
#include <QStringList>

#include "io/virtualfilesystem.h"

class QFile;

namespace EvilTemple {

class TroikaArchiveEntry;

class TroikaArchive : public QObject, public VirtualFileSystemHandler
{
Q_OBJECT
public:
    // Typedef for a list of directory entries
    typedef const QList<const TroikaArchiveEntry*> &Entries;

    explicit TroikaArchive(const QString &filename, QObject *parent = 0);
    ~TroikaArchive();

    QString filename() {
        return file->fileName();
    }

    // VirtualFileSystemHandler interface
    QByteArray openFile(const QString &filename);
    bool exists(const QString &filename);
    QStringList listFiles(const QString &path, const QString &filter = "*");

private:
    QVector<TroikaArchiveEntry> entries;
    QList<const TroikaArchiveEntry*> rootEntries;
    QFile *file;
    void readIndex();
    void readIndexEntry(QDataStream &dataStream, TroikaArchiveEntry &entry);
    const TroikaArchiveEntry *findEntry(const QString &filename) const;

    Q_DISABLE_COPY(TroikaArchive)
};

}

#endif // TROIKAARCHIVE_H
