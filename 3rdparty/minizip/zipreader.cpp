#include "zipreader.h"

class ZipReaderData
{
public:
    ZipReaderData(const QString &filename)
    {

    }

    ~ZipReaderData()
    {

    }
};

ZipReader::ZipReader(const QString &filename) : d_ptr(new ZipReaderData(filename))
{
}

ZipReader::~ZipReader()
{
}

QByteArray ZipReader::readFile(FileHandle handle)
{
    return 0;
}
