
#include <common/paths.h>

#include <iostream>

#include <QString>
#include <QScopedArrayPointer>
#include <QFile>
#include <QXmlStreamReader>

/**
  This rather messy include is here so we have access to the file format version
  supported by the converter this launcher is built with.
  */
#include "../conversion/include/conversion/version.h"

#if defined(GOOGLE_BREAKPAD_ENABLED)
#include "client/windows/handler/exception_handler.h"
#include <string>

static bool etFilterCallback(void *context, EXCEPTION_POINTERS *exinfo, MDRawAssertionInfo *assertion)
{
    return true;
}

static bool etMinidumpCallback(const wchar_t* dump_path, const wchar_t* minidump_id, void* context,
                               EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
{
    QString message = QString("A crash dump has been written to %1.dmp Please send this file to the developers.")
                      .arg(QString::fromWCharArray(minidump_id));

    QScopedArrayPointer<wchar_t> rawMessage(new wchar_t[message.length()+1]);
    message.toWCharArray(rawMessage.data());
    rawMessage[message.length()] = 0;

    MessageBoxW(NULL, rawMessage.data(), L"Crash Dump Written", MB_OK|MB_ICONERROR);
    return true;
}

static void install_google_breakpad()
{
    using namespace google_breakpad;
    using namespace std;

    new ExceptionHandler(wstring(L"."),
                         etFilterCallback,
                         etMinidumpCallback,
                         NULL,
                         ExceptionHandler::HANDLER_ALL,
                         MiniDumpNormal,
                         NULL,
                         NULL);
}

#endif

#include <QtGui>

#include <game.h>
#include <common/datafileengine.h>
#include <mainwindow.h>

using namespace EvilTemple;

static QFile logFile;

static bool outputToConsole = false;

enum VerifyResult {
    Verify_Failed,
    Verify_VersionMismatch,
    Verify_NotFound,
    Verify_Okay
};

VerifyResult verifyConversion(const Paths &paths);

static void messageHandler(QtMsgType type, const char *message)
{
    const char *prefix = NULL;

    switch (type) {
    case QtDebugMsg:
        prefix = "[DEBUG]";
        break;
    case QtWarningMsg:
        prefix = "[WARN]";
        break;
    case QtCriticalMsg:
        prefix = "[ERROR]";
        break;
    case QtFatalMsg:
        prefix = "[FATAL]";
        break;
    }

    if (logFile.isOpen()) {
        logFile.write(prefix);
        logFile.write(" ");
        logFile.write(message);
        logFile.write("\n");
        logFile.flush();
    }

    if (outputToConsole)
        printf("%s %s\n", prefix, message);

    if (type == QtFatalMsg)
        abort();
}

int main(int argc, char *argv[])
{
#if defined(GOOGLE_BREAKPAD_ENABLED)
    install_google_breakpad();
#endif

    /*
      We have to do this before actually instantiating QApplication, sicne that by itself may trigger
      log outputs. Use argv/argc scanning ONLY for this option, all other options may contain unicode
      characters and should therefore use QCoreApplication::arguments(), which takes Unicode into account.
     */
    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "-console")) {
            outputToConsole = true;
            std::cout << "Logging to console enabled." << std::endl;
        }
    }

    qInstallMsgHandler(messageHandler);

    QApplication a(argc, argv);

    Paths paths;

    logFile.setFileName(paths.userDataPath() + "eviltemple.log");

    if (!logFile.open(QIODevice::Text|QIODevice::Truncate|QIODevice::WriteOnly)) {
        qFatal("Unable to open logfile eviltemple.log for writing, please make sure "
               "that the current directory is writeable.");
    }

    // Set default values for org+app so QSettings uses them everywhere
    QCoreApplication::setOrganizationName("Sebastian Hartte");
    QCoreApplication::setOrganizationDomain("toee.hartte.de");
    QCoreApplication::setApplicationName("EvilTemple");

    DataFileEngineHandler dataFileHandler(paths.installationPath() + "data");

    // Add all archives in the generated data directory
    QStringList archives = paths.generatedDataDir().entryList(QStringList() << "*.zip", QDir::Files|QDir::Readable);

    VerifyResult verifyResult;
    while ((verifyResult = verifyConversion(paths)) != Verify_Okay) {
        int result = QMessageBox::question(NULL, "Error", "Before playing, data from the original Temple of "
                                           "Elemental Evil game needs to be converted to be used with this engine.\n\n"
                                           "Would you like to run the data converter now?",
                                           QMessageBox::Yes, QMessageBox::No);

        if (result == QMessageBox::Yes) {
            // Assume the same directory (DO NOT assume CWD!)
            QString converterPath = a.applicationDirPath() + QDir::separator() + "converter.exe";
            qDebug("Launching %s", qPrintable(converterPath));

            int retVal = QProcess::execute(converterPath);
            if (retVal < 0) {
                qDebug("Converter did not run successfully or was cancelled (Code: %d).", retVal);
                return -1;
            }

            qDebug("Converted exited with code: %d. Trying to resume startup.", retVal);

            // Try again if the process ended successfully
            archives = paths.generatedDataDir().entryList(QStringList() << "*.zip", QDir::Files|QDir::Readable);
            continue;
        }

        return -1;
    }

    foreach (const QString &archive, archives) {
        dataFileHandler.addArchive(paths.generatedDataDir().absoluteFilePath(archive));
    }

    Game game(&paths);

    if (!game.start()) {
        return EXIT_FAILURE;
    }

    MainWindow mainWindow(&game);
    mainWindow.readSettings();
    mainWindow.showFromSettings();

    return a.exec();
}

static VerifyResult verifyConversion(const Paths &paths)
{
    QString filename = paths.generatedDataPath() + "conversion.xml";

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug("Unable to find conversion report.");
        return Verify_NotFound;
    }

    QXmlStreamReader reader(&file);

    if (!reader.readNextStartElement()) {
        qDebug("Unable to read start element in conversion report.");
        return Verify_Failed;
    }

    if (!reader.attributes().hasAttribute("version")) {
        qDebug("Version attribute missing in conversion report.");
        return Verify_Failed;
    }

    QString versionString = reader.attributes().value("version").toString();

    bool ok;
    int version = versionString.toInt(&ok);

    if (!ok) {
        qDebug("Version not convertable to int in conversion report: %s", qPrintable(versionString));
        return Verify_Failed;
    }

    if (version != DataFormatVersion) {
        qDebug("File format version mismatch: %d != %d", version, DataFormatVersion);
        return Verify_VersionMismatch;
    }

    qDebug("Current data file version: %d", version);
    return Verify_Okay;

}
