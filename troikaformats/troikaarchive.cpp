
#include <QDataStream>
#include <QFile>
#include <QtGlobal>

#include "troikaarchive.h"

namespace Troika {

    /*
      Structure used to store entries in the virtual file system of an archive.
     */
    class TroikaArchiveEntry {
    public:

        QString path() const {
            QString result(filename);

            for (const TroikaArchiveEntry *parent = this->parent; parent; parent = parent->parent)
                result.prepend(parent->filename + "/");

            if (isDirectory())
                result.append('/');

            return result;
        }

        QList<const TroikaArchiveEntry*> children() const {
            QList<const TroikaArchiveEntry*> result;

            for (const TroikaArchiveEntry *entry = this->firstChild; entry; entry = entry->nextSibling)
                result.append(entry);

            return result;
        }

        bool isDirectory() const {
            return type == Directory;
        }

        bool isCompressed() const {
            return type == CompressedFile;
        }

        bool isFile() const {
            return (type == File || type == CompressedFile);
        }

        quint32 type;
        quint32 uncompressedSize;
        quint32 compressedSize;
        quint32 dataStart;
        QString filename;
        const TroikaArchiveEntry *parent;
        const TroikaArchiveEntry *nextSibling;
        const TroikaArchiveEntry *firstChild;

        enum Type {
            File = 1,
            CompressedFile = 2,
            Directory = 1024
        };
    };

    TroikaArchive::TroikaArchive(const QString &filename, QObject *parent) :
        QObject(parent),
        file(new QFile(filename, this))
    {
        if (file->open(QFile::ReadOnly)) {
            readIndex();
        } else {
            qWarning("Cannot open file: %s", qPrintable(file->fileName()));
        }
    }

    TroikaArchive::~TroikaArchive()
    {
        if (file->isOpen()) {
            file->close();
        }
    }

    void TroikaArchive::readIndex() {
        quint32 fsLength; // Length of the archive virtual file-system in byte
        quint32 entryCount; // Number of entries in archive file-system

        QDataStream dataStream(file);
        dataStream.setByteOrder(QDataStream::LittleEndian);

        file->seek(file->size() - 4); // Last 4 byte of file are file-system size (including those 4 byte)
        dataStream >> fsLength;

        Q_ASSERT(fsLength <= file->size()); // The fs table can span the entire archive

        file->seek(file->size() - fsLength);

        dataStream >> entryCount;

        entries.resize(entryCount); // Pre-allocate elements

        for (quint32 i = 0; i < entryCount; ++i) {
            readIndexEntry(dataStream, entries[i]);

            // Record the entries in the root directory as a starting point
            if (!entries[i].parent)
                rootEntries.append(entries.data() + i);
        }
    }

    // Reads a single entry from the archive file system
    void TroikaArchive::readIndexEntry(QDataStream &dataStream, TroikaArchiveEntry &entry) {
        QByteArray asciiFilename;
        dataStream >> asciiFilename;
        entry.filename = asciiFilename; // Pre-Convert to internal UCS-2 format here

        qint32 unknown, parentDirId, firstChildId, nextSiblingId;

        // Read all data for entry
        dataStream >> unknown >> entry.type >> entry.uncompressedSize >> entry.compressedSize >> entry.dataStart
                >> parentDirId >> firstChildId >> nextSiblingId;

        // Convert id's to pointers for faster traversal
        entry.parent = (parentDirId != -1) ? (entries.data() + parentDirId) : NULL;
        entry.nextSibling = (nextSiblingId != -1) ? (entries.data() + nextSiblingId) : NULL;
        entry.firstChild = (firstChildId != -1) ? (entries.data() + firstChildId) : NULL;

        Q_ASSERT(entry.isDirectory() || entry.firstChild == NULL); // Files mustn't have children
    }

    const TroikaArchiveEntry *TroikaArchive::findEntry(const QString &filename) const {
        QStringList parts = filename.split(QRegExp("[/\\\\]"), QString::SkipEmptyParts);

        const TroikaArchiveEntry *entry = NULL;
        QList<const TroikaArchiveEntry*> entries = rootEntries;

        foreach (QString part, parts) {
            const TroikaArchiveEntry *subentry = NULL;
            foreach (const TroikaArchiveEntry *entry, entries) {
                if (!entry->filename.compare(part, Qt::CaseInsensitive)) {
                    subentry = entry;
                    break;
                }
            }

            if (subentry == NULL) {
                return NULL;
            } else {
                entry = subentry;
                entries = entry->children();
            }
        }

        return entry;
    }

    QByteArray TroikaArchive::openFile(const QString &filename) {
        const TroikaArchiveEntry *entry = findEntry(filename);

        if (entry == NULL || !entry->isFile()) {
            return QByteArray((char*)NULL, 0);
        }

        readMutex.lock();

        file->seek(entry->dataStart);

        if (entry->isCompressed()) {
            QByteArray data(entry->compressedSize + sizeof(int), Qt::Uninitialized);
            QDataStream dataStream(&data, QIODevice::WriteOnly);
            dataStream << entry->uncompressedSize;
            file->read(data.data() + sizeof(int), entry->compressedSize);

            readMutex.unlock();

            return qUncompress(data);
        } else {
            QByteArray result = file->read(entry->uncompressedSize);
            readMutex.unlock();

            return result;
        }
    }

    bool TroikaArchive::exists(const QString &filename) {
        return findEntry(filename) != NULL;
    }

    QStringList TroikaArchive::listFiles(const QString &path, const QString &filter) {
        QStringList result;
        const TroikaArchiveEntry *entry = findEntry(path);

        if (!entry)
        {
            return result;
        }

        QString basepath = entry->path();

        QStringList filterParts = filter.split('*', QString::KeepEmptyParts);

        if (filterParts.count() > 2) {
            qWarning("listFiles may only have one wildcard per filter: %s.", qPrintable(filter));
        }

        foreach (const TroikaArchiveEntry *child, entry->children()) {
            if (filterParts.count() == 1 && !child->filename.compare(filter, Qt::CaseInsensitive)) {
                result.append(basepath + child->filename);
            } else if (child->filename.startsWith(filterParts[0], Qt::CaseInsensitive) &&
                       child->filename.endsWith(filterParts[1], Qt::CaseInsensitive)) {
                result.append(basepath + child->filename);
            }
        }

        return result;
    }

    QStringList TroikaArchive::listAllFiles(const QString &filter)
    {
        QStringList filterParts = filter.split('*', QString::KeepEmptyParts);

        if (filterParts.count() > 2) {
            qWarning("listFiles may only have one wildcard per filter: %s.", qPrintable(filter));
        }

        QStringList result;

        foreach (const TroikaArchiveEntry &entry, entries) {
            if (filterParts.count() == 1 && !entry.filename.compare(filter, Qt::CaseInsensitive)) {
                result.append(entry.path());
            } else if (entry.filename.startsWith(filterParts[0], Qt::CaseInsensitive) &&
                       entry.filename.endsWith(filterParts[1], Qt::CaseInsensitive)) {
                result.append(entry.path());
            }
        }

        return result;
    }

}
