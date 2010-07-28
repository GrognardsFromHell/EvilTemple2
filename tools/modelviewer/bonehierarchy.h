#ifndef BONEHIERARCHY_H
#define BONEHIERARCHY_H

#include <QDialog>
#include <QModelIndex>

#include "../../game/skeleton.h"
using namespace EvilTemple;

namespace Ui {
    class BoneHierarchy;
}


class BoneHierarchy : public QDialog
{
    Q_OBJECT

public:
    explicit BoneHierarchy(QWidget *parent = 0);
    ~BoneHierarchy();

    void setSkeleton(const Skeleton *skeleton);

signals:
    void selectBone(int boneId);

private:
    Ui::BoneHierarchy *ui;

private slots:
    void on_treeView_activated(QModelIndex index);
};

#endif // BONEHIERARCHY_H
