#ifndef CONVERSIONPAGE_H
#define CONVERSIONPAGE_H

#include <QWizardPage>
#include <QFile>

class QAxObject;

namespace Ui {
    class ConversionPage;
}

class ConversionThread;

class ConversionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ConversionPage(QWidget *parent = 0);
    ~ConversionPage();

    bool isComplete() const;

    void initializePage();
    void cleanupPage();

public slots:
    void updateProgress(int value, int max, const QString &operation);
    void addLogMessage(const QString &message);
    void threadStarted();
    void threadStopped();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ConversionPage *ui;
    ConversionThread *conversionThread;
    QFile logFile;
#ifdef Q_OS_WIN32
    QAxObject *mTaskbarList;
    bool mDontUseTaskbarList;
#endif
};

#endif // CONVERSIONPAGE_H
