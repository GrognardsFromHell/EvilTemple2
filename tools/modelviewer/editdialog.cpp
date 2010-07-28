#include "editdialog.h"
#include "ui_editdialog.h"

EditDialog::EditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDialog)
{
    ui->setupUi(this);

    on_scaleSlider_sliderMoved(ui->scaleSlider->value());
    on_rotationSlider_sliderMoved(ui->rotationSlider->value());
}

EditDialog::~EditDialog()
{
    delete ui;
}

void EditDialog::on_scaleSlider_sliderMoved(int position)
{
    ui->scaleXLabel->setText(QString::number(position / 100.0));
    emit scaleChanged(position / 100.0f);
}

void EditDialog::on_rotationSlider_sliderMoved(int position)
{
    ui->rotationLabel->setText(QString::number(position));
    emit rotationChanged(position);
}
