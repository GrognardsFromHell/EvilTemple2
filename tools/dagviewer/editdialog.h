#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>

namespace Ui {
    class EditDialog;
}

class EditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditDialog(QWidget *parent = 0);
    ~EditDialog();

signals:
    void scaleXChanged(float);
    void scaleYChanged(float);
    void scaleZChanged(float);
    void rotationChanged(float);

private:
    Ui::EditDialog *ui;

private slots:
    void on_rotationSlider_sliderMoved(int position);
    void on_scaleZSlider_sliderMoved(int position);
    void on_scaleYSlider_sliderMoved(int position);
    void on_scaleXSlider_sliderMoved(int position);
};

#endif // EDITDIALOG_H
