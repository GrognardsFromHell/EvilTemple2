#include <QtScript>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scripting.h"

using namespace EvilTemple;

static QScriptValue _scriptMessage(QScriptContext *context, QScriptEngine *engine, void *arg)
{
    if (context->argumentCount() > 0) {
        QScriptValue value = context->argument(0);
        ((MainWindow*)arg)->addScriptMessage(value.toString());
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

MainWindow::MainWindow(EvilTemple::AudioEngine *engine) : mEngine(engine),
    QMainWindow(NULL),
    ui(new Ui::MainWindow), scriptEngine(new QScriptEngine(this))
{
    ui->setupUi(this);

    registerAudioEngine(scriptEngine);

    // Register the audio engine with the script engine
    QScriptValue globalObject = scriptEngine->globalObject();
    globalObject.setProperty("audioEngine", scriptEngine->newQObject(mEngine));
    globalObject.setProperty("print", scriptEngine->newFunction(_scriptMessage, this));
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
    SharedSound sound(mEngine->readSound("sample.wav"));
    mEngine->playSound(sound, SoundCategory_Other);
}

void MainWindow::on_pushButton_2_clicked()
{
    mEngine->setPaused(ui->pushButton_2->isChecked());
}

void MainWindow::on_pushButton_3_clicked()
{
    QScriptValue value = scriptEngine->evaluate(ui->plainTextEdit->toPlainText());
    addScriptMessage(value.toString());
}

void MainWindow::addScriptMessage(const QString &message)
{
    ui->textBrowser->append(message + "\n");
}
