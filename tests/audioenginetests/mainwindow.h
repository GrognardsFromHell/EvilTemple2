#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtScript>
#include <QMainWindow>

#include "audioengine.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(EvilTemple::AudioEngine *engine);
    ~MainWindow();

    void addScriptMessage(const QString &message);
protected:
    void changeEvent(QEvent *e);

private:
    EvilTemple::AudioEngine *mEngine;
    Ui::MainWindow *ui;
    QScriptEngine *scriptEngine;

private slots:
    void on_pushButton_3_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
