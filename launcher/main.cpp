
#include <QString>
#include <QScopedArrayPointer>
#include <QFile>

#if defined(GOOGLE_BREAKPAD_ENABLED) && defined(Q_CC_MSVC)
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

#include "game.h"
#include "datafileengine.h"
#include "mainwindow.h"
#include "scriptengine.h"

using namespace EvilTemple;

static QFile logFile("eviltemple.log");

static bool outputToConsole = false;

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

    logFile.write(prefix);
    logFile.write(" ");
    logFile.write(message);
    logFile.write("\n");
    logFile.flush();

    if (outputToConsole)
        printf("%s %s\n", prefix, message);

    if (type == QtFatalMsg)
        abort();
}

int main(int argc, char *argv[])
{
#if defined(GOOGLE_BREAKPAD_ENABLED) && defined(Q_CC_MSVC)
    install_google_breakpad();
#endif

    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "-console")) {
            outputToConsole = true;
            printf("Logging to console enabled.\n");
        }
    }

    if (!logFile.open(QIODevice::Text|QIODevice::Truncate|QIODevice::WriteOnly)) {
        qFatal("Unable to open logfile eviltemple.log for writing, please make sure "
               "that the current directory is writeable.");
    }

    qInstallMsgHandler(messageHandler);

    QApplication a(argc, argv);

    // Set default values for org+app so QSettings uses them everywhere
    QCoreApplication::setOrganizationName("Sebastian Hartte");
    QCoreApplication::setOrganizationDomain("toee.hartte.de");
    QCoreApplication::setApplicationName("EvilTemple");

    DataFileEngineHandler dataFileHandler("data");

    Game game;

    if (!game.start()) {
        return EXIT_FAILURE;
    }

    MainWindow mainWindow(game);
    mainWindow.readSettings();
    mainWindow.showFromSettings();

    return a.exec();
}
