
#include <QDir>
#include <QList>
#include <QFile>
#include <QHash>
#include <QFSFileEngine>
#include <QDateTime>

#include <QMessageBox>

#include "unzip.h"
#include "common/datafileengine.h"

namespace EvilTemple {

class DataFileEngineHandlerData;

class DataFileEngine : public QAbstractFileEngine
{
public:
    DataFileEngine(DataFileEngineHandlerData *handler) : fileEngine(handler) {
    }

    void setFileName(const QString &file);
    Iterator *endEntryList();
    uchar *map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags);
    bool extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output);
    bool supportsExtension(Extension extension) const;
    bool open(QIODevice::OpenMode flags);
    bool close();
    bool flush();
    qint64 read(char *data, qint64 len);
    qint64 write(const char *, qint64);
    bool remove();
    bool copy(const QString &);
    bool rename(const QString &);
    bool link(const QString &);
    qint64 size() const;
    qint64 pos() const;
    bool atEnd() const;
    bool seek(qint64 pos);
    bool isSequential() const;
    FileFlags fileFlags(FileFlags type) const;
    bool setPermissions(uint);
    QString fileName(FileName file) const;
    bool isRelativePath() const;
    uint ownerId(FileOwner) const;
    QString owner(FileOwner) const;
    QDateTime fileTime(FileTime) const;
    Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames);

private:
    bool exists() const;
    QByteArray buffer;
    DataFileEngineHandlerData *fileEngine;
    QString currentFilename;
    int offset;
};

class OverrideFileEngine : public QFSFileEngine {
public:

    OverrideFileEngine(const QString &filename, const QString &originalFilename)
        : QFSFileEngine(filename), mOriginalFilename(originalFilename)
    {
    }

    void setFileName(const QString &file)
    {
        QFSFileEngine::setFileName(file);
        mOriginalFilename = file;
    }

    /**
      This fixes QFileInfo(path).isRelative() which is used by absoluteFilePath().
      The problem is that the data file engine converts relative paths to
      absolute ones silently, if the relative path exists in the data directory.

      This probably needs a better fix, i.e. store the original filename and operate
      on that, unless setFilename is called to set a new one.
      */
    bool isRelativePath() const
    {
        QFSFileEngine fileEngine(mOriginalFilename);
        return fileEngine.isRelativePath();
    }

private:
    QString mOriginalFilename;
};

class DataFileEngineHandlerData
{
public:
    DataFileEngineHandlerData(const QString &dataPath)
        : dataDir(dataPath)
    {
        if (!dataDir.makeAbsolute()) {
            qWarning("Unable to make the data path absolute.");
        }

        if (!dataDir.exists()) {
            qWarning("The data path %s doesn't exist.", qPrintable(dataDir.absolutePath()));
        }

        absoluteDataPath = dataDir.absolutePath();

        if (!absoluteDataPath.endsWith(QDir::separator())) {
            absoluteDataPath.append(QDir::separator());
        }

        absoluteDataPath = QDir::toNativeSeparators(absoluteDataPath);
    }

    ~DataFileEngineHandlerData()
    {
        foreach (unzFile archiveHandle, archiveHandles) {
            unzClose(archiveHandle);
        }
    }

    QAbstractFileEngine *create(const QString &filename)
    {
        QString path;

        // Filename must either be relative already, or in case its an absolute filename,
        // it must reside in the data directory.
        QFSFileEngine engine(filename);

        if (!engine.isRelativePath()) {
            QString nativePath = QDir::toNativeSeparators(filename); // Make sure all separators are native.
            if (nativePath.startsWith(absoluteDataPath)) {
                path = nativePath.right(nativePath.length() - absoluteDataPath.length());
            } else {
                return 0;
            }

        } else {
            path = QDir::toNativeSeparators(filename);
        }

        // We don't handle directories.
        if (!path.endsWith('.')) {
            // Look for the file in the data directory.
            engine.setFileName(absoluteDataPath + path);
            if (engine.fileFlags(QAbstractFileEngine::ExistsFlag) & QAbstractFileEngine::ExistsFlag) {
                return new OverrideFileEngine(absoluteDataPath + path, filename);
            }
        } else {
            return 0;
        }

        ArchiveEntries::const_iterator it = archiveEntries.find(path.toLower());

        if (it != archiveEntries.end()) {
            DataFileEngine *result = new DataFileEngine(this);
            result->setFileName(path);
            //qDebug("Loading %s from zip archive.", qPrintable(path));
            return result;
        }

        return 0;
    }

    size_t size(const QString &filename) const
    {
        ArchiveEntries::const_iterator entry = archiveEntries.find(filename.toLower());

        if (entry == archiveEntries.end()) {
            return 0;
        } else {
            return entry->uncompressedSize;
        }
    }

    bool exists(const QString &filename)
    {
        return archiveEntries.find(filename.toLower()) != archiveEntries.end();
    }

    QByteArray getArchiveFile(const QString &filename)
    {
        ArchiveEntries::const_iterator entry = archiveEntries.find(QDir::toNativeSeparators(filename).toLower());

        if (entry == archiveEntries.end()) {
            return 0;
        } else {
            int error = unzGoToFilePos64(entry->handle, &entry->position);

            if (error != UNZ_OK) {
                qWarning("Unable to seek to %s.", qPrintable(filename));
                return 0;
            }

            error = unzOpenCurrentFile(entry->handle);

            if (error != UNZ_OK) {
                qWarning("Unable to open file %s for reading in archive.", qPrintable(filename));
                return 0;
            }

            QByteArray result(entry->uncompressedSize, Qt::Uninitialized);

            error = unzReadCurrentFile(entry->handle, result.data(), result.size());

            unzCloseCurrentFile(entry->handle);

            if (error == UNZ_OK) {
                qWarning("Unable to read file from ZIP: %s.", qPrintable(filename));
                return 0;
            }

            return result;
        }
    }

    /**
      Loads all ZIP files in the data path.
      @returns The number of zip files that have been loaded.
      */
    int addArchives(const QDir &archiveDir)
    {
        QStringList zipEntries = archiveDir.entryList(QStringList() << "*.zip", QDir::Files);

        qDebug("Loading %d archives in %s.", zipEntries.size(), qPrintable(archiveDir.absolutePath()));

        int count = 0;

        foreach (const QString &zipEntry, zipEntries) {
            QString archiveFilename = archiveDir.absoluteFilePath(zipEntry);
            if (addArchive(archiveFilename))
                count++;
        }

        return count;
    }

    bool addArchive(const QString &archiveFilename)
    {
        unzFile archiveHandle = unzOpen64(qPrintable(archiveFilename));

        if (!archiveHandle) {
            qWarning("Unable to open ZIP archive %s.", qPrintable(archiveFilename));
            return false;
        }

        archiveHandles.append(archiveHandle); // Ensures closing of archives

        int error;

        // Retrieve some global information about the archive
        unz_global_info archiveInfo;
        error = unzGetGlobalInfo(archiveHandle, &archiveInfo);

        if (error != UNZ_OK)  {
            qWarning("Unable to retrieve global information from zip archive %s.", qPrintable(archiveFilename));
            return false;
        }

        qDebug("Loading ZIP archive %s (%lu entries).", qPrintable(archiveFilename), archiveInfo.number_entry);

        error = unzGoToFirstFile(archiveHandle);

        if (error != UNZ_OK) {
            qWarning("Unable to seek to first file in ZIP archive %s.", qPrintable(archiveFilename));
            return false;
        }

        // Small performance optimization to resize the hashtable ahead of time
        archiveEntries.reserve(archiveEntries.size() + archiveInfo.number_entry);

        unz_file_info64 fileInfo;
        unz64_file_pos filePos; // Handle to the file in the archive
        QByteArray filenameBuffer;

        do {
            error = unzGetCurrentFileInfo64(archiveHandle, &fileInfo, NULL, 0, NULL, 0, NULL, 0);

            if (error != UNZ_OK) {
                qWarning("Unable to read local file info from archive %s.", qPrintable(archiveFilename));
                continue;
            }

            filenameBuffer.resize(fileInfo.size_filename);
            error = unzGetCurrentFileInfo64(archiveHandle, &fileInfo, filenameBuffer.data(), fileInfo.size_filename,
                                            NULL, 0, NULL, 0);

            if (error != UNZ_OK) {
                qWarning("Unable to read filename from archive %s.", qPrintable(archiveFilename));
                continue;
            }

            QString filename = QString::fromLocal8Bit(filenameBuffer, fileInfo.size_filename);

            error = unzGetFilePos64(archiveHandle, &filePos);

            if (error != UNZ_OK) {
                qWarning("Couldn't get position of %s in archive %s.", qPrintable(filename),
                         qPrintable(archiveFilename));
                continue;
            }

            // An entry describes in which archive the file is stored and where.
            ArchiveEntry entry;
            entry.handle = archiveHandle;
            entry.position = filePos;
            entry.uncompressedSize = fileInfo.uncompressed_size;
            entry.compressedSize = fileInfo.compressed_size;
            entry.compressionMethod = fileInfo.compression_method;

            archiveEntries[QDir::toNativeSeparators(filename).toLower()] = entry;

            error = unzGoToNextFile(archiveHandle);
        } while (error == UNZ_OK);

        if (error != UNZ_END_OF_LIST_OF_FILE) {
            qWarning("Unable to seek to next file in ZIP archive %s.", qPrintable(archiveFilename));
            return false;
        }

        return true;
    }

private:
    QDir dataDir;
    QString absoluteDataPath;

    QList<unzFile> archiveHandles; // Close archives when done

    struct ArchiveEntry {
        unzFile handle;
        unz64_file_pos position;
        size_t uncompressedSize;
        size_t compressedSize;
        size_t compressionMethod;
    };

    typedef QHash<QString, ArchiveEntry> ArchiveEntries;

    ArchiveEntries archiveEntries; // Archive entries (lowercase filename -> archive pos)

};

DataFileEngineHandler::DataFileEngineHandler(const QString &dataPath)
    : d_ptr(0)
{
    DataFileEngineHandlerData *data = new DataFileEngineHandlerData(dataPath);
    d_ptr.reset(data); // During the initialization of the handler, d_ptr MUST be null
}

DataFileEngineHandler::~DataFileEngineHandler()
{
}

bool DataFileEngineHandler::addArchive(const QString &filename)
{
    return d_ptr->addArchive(filename);
}

int DataFileEngineHandler::addArchives(const QString &directory)
{
    return d_ptr->addArchives(directory);
}

QAbstractFileEngine *DataFileEngineHandler::create(const QString &fileName) const
{
    if (d_ptr) {
        return d_ptr->create(fileName);
    } else {
        return 0;
    }
}

void DataFileEngine::setFileName(const QString &file)
{
    currentFilename = file;
}

bool DataFileEngine::open(QIODevice::OpenMode flags)
{
    if (currentFilename.isEmpty()) {
        qWarning("QResourceFileEngine::open: Missing file name");
        return false;
    }
    if(flags & QIODevice::WriteOnly)
        return false;
    if(!exists())
       return false;
    buffer = fileEngine->getArchiveFile(currentFilename);
    offset = 0;
    return true;
}

bool DataFileEngine::exists() const
{
    return !buffer.isNull() || fileEngine->exists(currentFilename);
}

bool DataFileEngine::close()
{
    offset = 0;
    buffer.clear();
    Q_ASSERT(buffer.isNull());
    return true;
}

bool DataFileEngine::flush()
{
    return true;
}

qint64 DataFileEngine::read(char *data, qint64 len)
{
    if(len > size() - offset)
        len = size() - offset;
    if(len <= 0)
        return 0;
    memcpy(data, buffer.data() + offset, len);
    offset += len;
    return len;
}

qint64 DataFileEngine::write(const char *, qint64)
{
    return -1;
}

bool DataFileEngine::remove()
{
    return false;
}

bool DataFileEngine::copy(const QString &)
{
    return false;
}

bool DataFileEngine::rename(const QString &)
{
    return false;
}

bool DataFileEngine::link(const QString &)
{
    return false;
}

qint64 DataFileEngine::size() const
{
    if(!exists())
        return 0;
    return fileEngine->size(currentFilename);
}

qint64 DataFileEngine::pos() const
{
    return offset;
}

bool DataFileEngine::atEnd() const
{
    if(!exists())
        return true;
    return offset == size();
}

bool DataFileEngine::seek(qint64 pos)
{
    if(!exists())
        return false;

    if(offset > size())
        return false;
    offset = pos;
    return true;
}

bool DataFileEngine::isSequential() const
{
    return false;
}

QAbstractFileEngine::FileFlags DataFileEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
    QAbstractFileEngine::FileFlags ret = 0;
    if(!exists())
        return ret;

    if(type & PermsMask)
        ret |= QAbstractFileEngine::FileFlags(ReadOwnerPerm|ReadUserPerm|ReadGroupPerm|ReadOtherPerm);
    if(type & TypesMask) {
        ret |= FileType; // Directories are not mapped correctly
    }
    if(type & FlagsMask) {
        ret |= ExistsFlag;
    }
    return ret;
}

bool DataFileEngine::setPermissions(uint)
{
    return false;
}

QString DataFileEngine::fileName(FileName file) const
{
    if(file == BaseName) {
        int slash = currentFilename.lastIndexOf(QLatin1Char('/'));
        if (slash == -1)
            return currentFilename;
        return currentFilename.mid(slash + 1);
    } else if(file == PathName || file == AbsolutePathName) {
        const int slash = currentFilename.lastIndexOf(QLatin1Char('/'));
        if (slash != -1)
            return currentFilename.left(slash);
    } else if(file == CanonicalName || file == CanonicalPathName) {
        const QString absoluteFilePath = currentFilename;
        if(file == CanonicalPathName) {
            const int slash = absoluteFilePath.lastIndexOf(QLatin1Char('/'));
            if (slash != -1)
                return absoluteFilePath.left(slash);
        }
        return absoluteFilePath;
    }
    return currentFilename;
}

bool DataFileEngine::isRelativePath() const
{
    return false;
}

uint DataFileEngine::ownerId(FileOwner) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

QString DataFileEngine::owner(FileOwner) const
{
    return QString();
}

QDateTime DataFileEngine::fileTime(FileTime) const
{
    return QDateTime();
}

QAbstractFileEngine::Iterator *DataFileEngine::beginEntryList(QDir::Filters filters,
                                                                   const QStringList &filterNames)
{
    // return new QResourceFileEngineIterator(filters, filterNames);
    return 0; // ...
}

QAbstractFileEngine::Iterator *DataFileEngine::endEntryList()
{
    return 0;
}

bool DataFileEngine::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
    if (extension == MapExtension) {
        const MapExtensionOption *options = (MapExtensionOption*)(option);
        MapExtensionReturn *returnValue = static_cast<MapExtensionReturn*>(output);
        returnValue->address = map(options->offset, options->size, options->flags);
        return (returnValue->address != 0);
    }
    if (extension == UnMapExtension) {
        return true; // No unmapping needs to be done.
    }
    return false;
}

uchar *DataFileEngine::map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags)
{
    Q_UNUSED(flags);
    if (offset < 0 || size <= 0 || !exists() || offset + size > buffer.size()) {
        setError(QFile::UnspecifiedError, QString());
        return 0;
    }
    uchar *address = reinterpret_cast<uchar *>(buffer.data());
    return (address + offset);
}

bool DataFileEngine::supportsExtension(Extension extension) const
{
    return (extension == UnMapExtension || extension == MapExtension);
}

}
