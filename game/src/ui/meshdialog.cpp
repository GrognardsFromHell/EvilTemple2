
#include "ui/meshdialog.h"
#include "ui_meshdialog.h"
#include "geometrymeshobject.h"
#include "model.h"

namespace EvilTemple
{

    QTreeWidgetItem *createItem(const QVector<Bone> &bones, const Bone &bone)
    {
        QStringList columns;
        columns.append(bone.name);
        QTreeWidgetItem *item = new QTreeWidgetItem(columns);

        foreach (int childId, bone.childrenIds)
            item->addChild(createItem(bones, bones[childId]));

        return item;
    }

    MeshDialog::MeshDialog(GeometryMeshObject *mesh, QWidget *parent) :
            QDialog(parent),
            ui(new Ui::MeshDialog)
    {
        ui->setupUi(this);

        const QBox3D &boundingBox = mesh->boundingBox();
        ui->boundingBox->setText(QString("%1,%2,%3 -> %4,%5,%6")
                                 .arg(boundingBox.minimum().x())
                                 .arg(boundingBox.minimum().y())
                                 .arg(boundingBox.minimum().z())
                                 .arg(boundingBox.maximum().x())
                                 .arg(boundingBox.maximum().y())
                                 .arg(boundingBox.maximum().z()));

        if (!mesh->mesh())
            return;

        QList<QTreeWidgetItem*> items;
        const Skeleton *skeleton = mesh->mesh()->skeleton();

        foreach (const Bone &bone, skeleton->bones())
        {
            if (bone.parentId == -1)
                items.append(createItem(skeleton->bones(), bone));
        }

        ui->boneTree->addTopLevelItems(items);

        // Add animations to dialog
        QList<QTreeWidgetItem*> tableItems;

        foreach (const Animation &animation, skeleton->animations()) {
            QTreeWidgetItem *item = new QTreeWidgetItem;

            item->setText(0, animation.name());

            switch (animation.driveType()) {
            case Animation::Distance:
                item->setText(1, "Distance");
                break;
            case Animation::Rotation:
                item->setText(1, "Rotation");
                break;
            case Animation::Time:
                item->setText(1, "Time");
                break;
            }

            item->setText(2, QString("%1").arg(animation.frames()));
            item->setText(3, QString("%1").arg(animation.frameRate(), 0, 'g', 2));

            tableItems.append(item);
        }

        ui->animationTree->addTopLevelItems(tableItems);
        ui->animationTree->setColumnWidth(0, 200);
        ui->animationTree->setColumnWidth(1, 100);
        ui->animationTree->setColumnWidth(2, 100);
        ui->animationTree->setColumnWidth(3, 100);
    }       

    MeshDialog::~MeshDialog()
    {
        delete ui;
    }

    void MeshDialog::changeEvent(QEvent *e)
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

}
