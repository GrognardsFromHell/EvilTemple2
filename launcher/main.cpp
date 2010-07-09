
#include <QString>
#include <QScopedArrayPointer>

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

    ExceptionHandler *handler = new ExceptionHandler(wstring(L"."),
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

int main(int argc, char *argv[])
{
#if defined(GOOGLE_BREAKPAD_ENABLED)
    install_google_breakpad();
#endif

    QApplication a(argc, argv);

    // Set default values for org+app so QSettings uses them everywhere
    QCoreApplication::setOrganizationName("Sebastian Hartte");
    QCoreApplication::setOrganizationDomain("toee.hartte.de");
    QCoreApplication::setApplicationName("EvilTemple");

    DataFileEngineHandler dataFileHandler("data");

    Game game;

    if (!game.start()) {
        return -1;
    }

    MainWindow mainWindow(game);
    mainWindow.readSettings();
    mainWindow.showFromSettings();

    return a.exec();
}
