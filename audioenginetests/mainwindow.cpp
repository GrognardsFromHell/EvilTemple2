#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "wavereader.h"
#include "mp3reader.h"

using namespace EvilTemple;

MainWindow::MainWindow(EvilTemple::AudioEngine *engine) : mEngine(engine),
    QMainWindow(NULL),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButton_clicked()
{
    SharedSound sound(WaveReader::read("sample.wav"));
    mEngine->playSound(sound.data());
}

void MainWindow::on_pushButton_2_clicked()
{
    mEngine->setPaused(ui->pushButton_2->isChecked());
}
