#ifndef PROFILERDIALOG_H
#define PROFILERDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

namespace EvilTemple {

namespace Ui {
    class ProfilerDialog;
}

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
};

}

#endif // PROFILERDIALOG_H
