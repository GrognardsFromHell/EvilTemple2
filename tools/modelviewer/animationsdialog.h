#ifndef ANIMATIONSDIALOG_H
#define ANIMATIONSDIALOG_H

#include <QDialog>
#include "animinfo.h"

namespace Ui {
    class AnimationsDialog;
}

class AnimationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AnimationsDialog(QWidget *parent = 0);
    ~AnimationsDialog();

    void setAnimations(const QList<AnimationInfo> &animations);

signals:
    void playAnimation(const QString &name);

private:
    Ui::AnimationsDialog *ui;

private slots:
    void on_pushButton_clicked();
};

#endif // ANIMATIONSDIALOG_H
