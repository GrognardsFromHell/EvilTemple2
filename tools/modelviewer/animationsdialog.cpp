#include "animationsdialog.h"
#include "ui_animationsdialog.h"

AnimationsDialog::AnimationsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnimationsDialog)
{
    ui->setupUi(this);
}

AnimationsDialog::~AnimationsDialog()
{
    delete ui;
}

void AnimationsDialog::setAnimations(const QList<AnimationInfo> &animations)
{
    ui->tableWidget->clear();

    ui->tableWidget->setColumnCount(7);
    ui->tableWidget->setRowCount(animations.size());

    QStringList headers;
    headers << "Name" << "Frames" << "Driven by" << "FPS" << "DPS" << "Affected Bones" << "Events";

    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    for (int i = 0; i < animations.size(); ++i) {
        QTableWidgetItem *item;

        item = new QTableWidgetItem(animations[i].name);
        ui->tableWidget->setItem(i, 0, item);

        item = new QTableWidgetItem(QString::number(animations[i].frames));
        ui->tableWidget->setItem(i, 1, item);

        item = new QTableWidgetItem(animations[i].driveType);
        ui->tableWidget->setItem(i, 2, item);

        item = new QTableWidgetItem(QString::number(animations[i].fps));
        ui->tableWidget->setItem(i, 3, item);

        item = new QTableWidgetItem(QString::number(animations[i].dps));
        ui->tableWidget->setItem(i, 4, item);

        item = new QTableWidgetItem(QString::number(animations[i].affectedBones));
        ui->tableWidget->setItem(i, 5, item);

        item = new QTableWidgetItem(QString::number(animations[i].events.size()));
        ui->tableWidget->setItem(i, 6, item);
    }
}

void AnimationsDialog::on_pushButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    emit playAnimation(ui->tableWidget->item(row, 0)->text());
}
