
#include <QThread>
#include <QTextStream>
#include <QUuid>
#include <QDir>

#include <common/paths.h>
#include <conversion/converter.h>

#include "conversionpage.h"
#include "ui_conversionpage.h"

#if defined(Q_OS_WIN32) && defined(Q_CC_MSVC)
#include <QAxObject>
#include <windows.h>
#include "shobjidl.h"

#include "ntverp.h"

// Only use the taskbar list if the correct minimum platform SDK is installed
#if VER_PRODUCTBUILD >= 7600
#define USE_TASKBAR_LIST
#endif

#endif

static QtMsgHandler oldMsgHandler = NULL;

static QTextBrowser *logBrowser = NULL;

static QTextStream *logStream = NULL;

static void handleQtMessage(QtMsgType type, const char *msg)
{
    QString text = QString::fromLocal8Bit(msg);

    switch (type) {
    case QtDebugMsg:
        text.prepend("[DEBUG] ");
        break;
    case QtWarningMsg:
        text.prepend("[WARN] ");
        break;
    case QtCriticalMsg:
        text.prepend("[CRITICAL] ");
        break;
    case QtFatalMsg:
        text.prepend("[FATAL] ");
        break;
    }

    if (logStream) {
        (*logStream) << text << endl;
    }

    if (logBrowser) {
        // Appending cannot be done in the same thread
        QMetaObject::invokeMethod(logBrowser, "append", Qt::QueuedConnection, Q_ARG(QString, text));
    }

    if (oldMsgHandler)
        oldMsgHandler(type, msg);
    else
        printf("%s\n", qPrintable(text));
}

class ConversionThread : public QThread {
public:
    ConversionThread() : mPage(NULL), mConverter(NULL)
    {
    }

    ~ConversionThread()
    {
        delete mConverter;
    }

    void cancel()
    {
        if (mConverter)
            mConverter->cancel();
    }

    void setDataPath(const QString &dataPath);
    void setOutputPath(const QString &outputPath);
    void setConversionPage(ConversionPage *page);

protected:
    void run();

private:
    QString mDataPath;
    QString mOutputPath;
    ConversionPage *mPage;
    Converter *mConverter;
};

inline void ConversionThread::setConversionPage(ConversionPage *page)
{
    mPage = page;
}

inline void ConversionThread::setDataPath(const QString &dataPath)
{
    mDataPath = dataPath;
}

inline void ConversionThread::setOutputPath(const QString &outputPath)
{
    mOutputPath = outputPath;
}

void ConversionThread::run()
{
    delete mConverter;
    mConverter = new Converter(mDataPath, mOutputPath);
    connect(mConverter, SIGNAL(progressUpdate(int,int,QString)), mPage, SLOT(updateProgress(int,int,QString)),
            Qt::QueuedConnection);
    if (!mConverter->convert()) {
        qWarning("Conversion was NOT successful.");
    }
}

ConversionPage::ConversionPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::ConversionPage),
    conversionThread(new ConversionThread)
#if defined(USE_TASKBAR_LIST)
    , mTaskbarList(0), mDontUseTaskbarList(false)
#endif
{
    ui->setupUi(this);
    setTitle("Converting resources");
    setSubTitle("Please wait while resources are converted. This may take several minutes.");
}

ConversionPage::~ConversionPage()
{
    delete ui;
}

void ConversionPage::changeEvent(QEvent *e)
{
    QWizardPage::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ConversionPage::updateProgress(int value, int max, const QString &operation)
{
    ui->progressBar->setMaximum(max);
    ui->progressBar->setValue(value);
    setSubTitle(operation);

#if defined(USE_TASKBAR_LIST)
    if (mDontUseTaskbarList)
        return;

    static ITaskbarList3 *taskbarList = NULL;

    if (!mTaskbarList) {
        mTaskbarList = new QAxObject;
        if (!mTaskbarList->setControl("{56FDF344-FD6D-11d0-958A-006097C9A090}")) {
            qDebug("Failed to create tasklist3. Not using it.");
            mDontUseTaskbarList = true;
            return;
        }
        mTaskbarList->disableMetaObject();
    }

    if (!taskbarList) {
        QUuid uuid("{ea1afb91-9e28-4b86-90e9-9e9f8a5eefaf}");
        if (mTaskbarList->queryInterface(uuid, (void**)&taskbarList) != S_OK) {
            qDebug("Unable to quer itaskbarlist3 interface. Skipping");
            mDontUseTaskbarList = true;
            return;
        }
    }

    QWidget *root = window();
    if (root) {
        taskbarList->SetProgressValue((HWND)root->winId(), value, max);
    }
#endif
}

static QString getOutputDirectory() {
    EvilTemple::Paths paths;

    return paths.generatedDataPath();
}

void ConversionPage::initializePage()
{
    updateProgress(0, 100, "Starting conversion");

    logBrowser = ui->textBrowser;
    oldMsgHandler = qInstallMsgHandler(handleQtMessage);

    logFile.setFileName("converter.log");

    if (logFile.open(QIODevice::Truncate|QIODevice::WriteOnly|QIODevice::Text)) {
        delete logStream;
        logStream = new QTextStream(&logFile);
    } else {
        qWarning("Unable to create conversion logfile converter.log: %s", qPrintable(logFile.errorString()));
    }

    connect(conversionThread, SIGNAL(started()), SLOT(threadStarted()));
    connect(conversionThread, SIGNAL(finished()), SLOT(threadStopped()));

    conversionThread->setDataPath(field("installationPath").toString());

    conversionThread->setOutputPath(getOutputDirectory());
    conversionThread->setConversionPage(this);

    conversionThread->start();
    emit completeChanged();
}

void ConversionPage::threadStarted()
{
    emit completeChanged();
}

void ConversionPage::threadStopped()
{
    emit completeChanged();
}

void ConversionPage::cleanupPage()
{
    if (conversionThread->isRunning()) {
        conversionThread->cancel();
        conversionThread->wait();
    }

    qInstallMsgHandler(oldMsgHandler);
    logBrowser = NULL;
    oldMsgHandler = NULL;
}

void ConversionPage::addLogMessage(const QString &message)
{
    ui->textBrowser->append(message);
}

bool ConversionPage::isComplete() const
{
    return conversionThread->isFinished();
}
