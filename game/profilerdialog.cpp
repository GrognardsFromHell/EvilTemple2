
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtGui/QStandardItem>
#include <QtGui/QTableView>

#include "profilerdialog.h"
#include "profiler.h"

#include "ui_profilerdialog.h"

namespace EvilTemple {

ProfilerDialog::ProfilerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProfilerDialog),
    model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Category, Sample Count, Total Time, Mean Time
    QStringList columns;
    columns << "Category" << "Sample Count" << "Total Time (ms)" << "Mean Time (ms)";
    model->setHorizontalHeaderLabels(columns);

    ui->tableView->setModel(model);

    QTimer *updateTimer = new QTimer(this);
    updateTimer->setInterval(1000);
    updateTimer->setSingleShot(false);
    connect(updateTimer, SIGNAL(timeout()), SLOT(updateData()));
    updateTimer->start();
}

ProfilerDialog::~ProfilerDialog()
{
    delete ui;
}

void ProfilerDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ProfilerDialog::updateData()
{
    Profiler::Report report = Profiler::report();

    model->setRowCount(0);

    const QString categoryNames[Profiler::Count] = {
        "SceneElapseTime",
        "ModelInstanceElapseTime",
        "ParticleSystemElapseTime",
        "ModelInstanceRender",
        "ParticleSystemRender",
        "SceneRender"
    };

    for (int i = 0; i < Profiler::Count; ++i) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(categoryNames[i]));
        row.append(new QStandardItem(QString("%1").arg(report.totalSamples[i])));
        row.append(new QStandardItem(QString("%1").arg(report.totalElapsedTime[i])));
        row.append(new QStandardItem(QString("%1").arg(report.meanTime[i])));
        model->appendRow(row);
    }

    ui->tableView->update();
}

}
