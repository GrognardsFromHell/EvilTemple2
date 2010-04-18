
#include <QtTest/QtTest>

#include "zipwriter.h"

#include "testzipwriter.h"
#include "unzip.h"

void TestZipWriter::emptyZipFile()
{
    QString tempPath = QDir::tempPath();
    if (!tempPath.endsWith(QDir::separator()))
        tempPath.append(QDir::separator());

    tempPath.append("test.zip");

    qDebug("Creating test file in %s.", qPrintable(tempPath));

    ZipWriter writer(tempPath);
    writer.addFile("test.txt", "testdata", 0);
    writer.close();

    QVERIFY2(QFile::exists(tempPath), "Zip file wasn't created.");

    unzFile uf = unzOpen64(tempPath.toLocal8Bit());

    if (!uf) {
        qErrnoWarning(errno, "Error code %d", errno);
    }

    QVERIFY2(uf != NULL, "Unable to open zip file with minizip.");

    unz_global_info64 globalInfo;
    QVERIFY2(unzGetGlobalInfo64(uf, &globalInfo) == UNZ_OK, "Cannot get global info for zip file.");

    QCOMPARE((int)globalInfo.number_entry, 0);

    QVERIFY2(unzClose(uf) == UNZ_OK, "Cannot close file.");

    // QFile::remove(tempPath);
}

QTEST_MAIN(TestZipWriter)
