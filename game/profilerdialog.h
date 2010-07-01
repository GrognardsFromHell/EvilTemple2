#ifndef PROFILERDIALOG_H
#define PROFILERDIALOG_H

#include <QObject>
#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
    class ProfilerDialog;
}

namespace EvilTemple {

class ProfilerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfilerDialog(QWidget *parent = 0);
    ~ProfilerDialog();

public slots:
    void updateData();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ProfilerDialog *ui;
    QStandardItemModel *model;

    uint lastReportedFrame;

private slots:
    void on_pushButton_clicked();
};

}

#endif // PROFILERDIALOG_H
