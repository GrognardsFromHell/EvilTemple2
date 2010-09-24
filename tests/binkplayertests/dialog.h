#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QImage>

namespace Ui {
    class Dialog;
}

class MyGlWidget;

class Dialog : public QDialog {
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
protected:
    void changeEvent(QEvent *e);
    void resizeEvent(QResizeEvent * event);
public slots:
    void showFrame(const QImage &frame);
private:
    Ui::Dialog *ui;
    MyGlWidget *glWidget;
};

#endif // DIALOG_H
