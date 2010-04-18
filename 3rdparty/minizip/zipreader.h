#ifndef ZIPREADER_H
#define ZIPREADER_H

#include <QString>
#include <QScopedPointer>

class ZipReaderData;

class ZipReader
{
public:
    ZipReader(const QString &filename);
    ~ZipReader();

    typedef void* FileHandle; // Handle to a single file in the archive, required to read the contents of a file

    QByteArray readFile(FileHandle handle);

    /**
      Used to iterate over the entries of an archive.
      */
    class iterator {

    };

private:
    QScopedPointer<ZipReaderData> d_ptr;
};

#endif // ZIPREADER_H
