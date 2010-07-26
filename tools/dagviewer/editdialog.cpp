#include "editdialog.h"
#include "ui_editdialog.h"

EditDialog::EditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDialog)
{
    ui->setupUi(this);

    on_scaleXSlider_sliderMoved(ui->scaleXSlider->value());
    on_scaleYSlider_sliderMoved(ui->scaleYSlider->value());
    on_scaleZSlider_sliderMoved(ui->scaleZSlider->value());
    on_rotationSlider_sliderMoved(ui->rotationSlider->value());
}

EditDialog::~EditDialog()
{
    delete ui;
}

void EditDialog::on_scaleXSlider_sliderMoved(int position)
{
    ui->scaleXLabel->setText(QString::number(position / 100.0));
    emit scaleXChanged(position / 100.0f);
}

void EditDialog::on_scaleYSlider_sliderMoved(int position)
{
    ui->scaleYLabel->setText(QString::number(position / 100.0));
    emit scaleYChanged(position / 100.0f);
}

void EditDialog::on_scaleZSlider_sliderMoved(int position)
{
    ui->scaleZLabel->setText(QString::number(position / 100.0));
    emit scaleZChanged(position / 100.0f);
}

void EditDialog::on_rotationSlider_sliderMoved(int position)
{
    ui->rotationLabel->setText(QString::number(position));
    emit rotationChanged(position);
}
