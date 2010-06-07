#ifndef CHOOSEDIRECTORYPAGE_H
#define CHOOSEDIRECTORYPAGE_H

#include <QWizardPage>

namespace Ui {
    class ChooseDirectoryPage;
}

class ChooseDirectoryPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ChooseDirectoryPage(QWidget *parent = 0);
    ~ChooseDirectoryPage();

    QString chosenDirectory() const;

    bool isComplete() const;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ChooseDirectoryPage *ui;

    bool mComplete;

private slots:
    void on_choosePath_clicked();
    void updateStatus();    
};

#endif // CHOOSEDIRECTORYPAGE_H
