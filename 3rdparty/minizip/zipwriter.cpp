
#include <QDateTime>

#include "zip.h"

#include "zipwriter.h"

class ZipWriterData
{
public:
    zipFile zFile;
};

ZipWriter::ZipWriter(const QString &filename) : d_ptr(new ZipWriterData)
{
    d_ptr->zFile = zipOpen64(filename.toLocal8Bit(), APPEND_STATUS_CREATE);

    if (!d_ptr->zFile) {
        qWarning("Unable to create zip file %s.", qPrintable(filename));
    }
}

ZipWriter::~ZipWriter()
{
    close(); // Force close if object is destroyed
}

static void setToCurrentDate(tm_zip *zipTime) {
    QDateTime now = QDateTime::currentDateTime();

    QTime nowTime = now.time();
    zipTime->tm_hour = nowTime.hour();
    zipTime->tm_min = nowTime.minute();
    zipTime->tm_sec = nowTime.second();

    QDate nowDate = now.date();
    zipTime->tm_year = nowDate.year();
    zipTime->tm_mon = nowDate.month()-1;
    zipTime->tm_mday = nowDate.day();
}

bool ZipWriter::addFile(const QString &filename, const QByteArray &data, int compressionLevel)
{
    QByteArray filenameInZip = filename.toLocal8Bit();

    // Filenames must not start with a slash, otherwise the ZIP file is not readable by WinXP
    while (filenameInZip.startsWith('/') || filenameInZip.startsWith('\\'))
        filenameInZip = filenameInZip.right(filenameInZip.size() - 1);

    zip_fileinfo fileInfo;
    memset(&fileInfo, 0, sizeof(fileInfo));
    setToCurrentDate(&fileInfo.tmz_date);

    int err = zipOpenNewFileInZip3_64(d_ptr->zFile, filenameInZip, &fileInfo,
                                     NULL,0,NULL,0,NULL /* comment*/,
                                     (compressionLevel != 0) ? Z_DEFLATED : 0,
                                     compressionLevel, 0,
                                     /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                     -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                     0, 0, 0);

    if (err != ZIP_OK) {
        qWarning("Unable to add %s to zip file. Error Code: %d", qPrintable(filename), err);
        return false;
    }

    err = zipWriteInFileInZip(d_ptr->zFile, data, data.size());

    if (err != ZIP_OK) {
        qWarning("Unable to write to %s in zip file. Error code: %d.", qPrintable(filename), err);
        return false;
    }

    err = zipCloseFileInZip(d_ptr->zFile);

    if (err != ZIP_OK) {
        qWarning("Unable to close %s in zip file. Error: %d", qPrintable(filename), err);
    }

    return true;
}

void ZipWriter::close()
{
    if (d_ptr->zFile) {
        int err = zipClose(d_ptr->zFile, 0);
        if (err != ZIP_OK) {
            qWarning("Unable to close zip file. Error code: %d.", err);
        }
        d_ptr->zFile = 0;
    }
}
