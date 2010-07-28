
#include <QAbstractItemModel>

#include "bonehierarchy.h"
#include "ui_bonehierarchy.h"


class SkeletonModel : public QAbstractItemModel
{
Q_OBJECT
public:
    SkeletonModel(const Skeleton *skeleton) : mSkeleton(skeleton)
    {
        foreach (const Bone *bone, skeleton->bones()) {
            if (!bone->parent())
                mRootBones.append(bone);
        }
    }

    int columnCount(const QModelIndex & parent) const
    {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (role == Qt::DisplayRole) {
            const Bone *bone = (const Bone*)index.internalPointer();
            return bone->name();
        } else {
            return QVariant();
        }
    }

    QModelIndex index(int row, int column, const QModelIndex &parent) const
    {
        if (!parent.isValid()) {
            if (row < mRootBones.size() && column == 0) {
                return createIndex(row, column, (void*)mRootBones[row]);
            } else {
                return QModelIndex();
            }
        }

        // If parent is valid, find children
        int childIndex = -1;
        const Bone *parentBone = (const Bone*)parent.internalPointer();
        foreach (const Bone *bone, mSkeleton->bones()) {
            if (bone->parent() == parentBone) {
                childIndex++;
                if (childIndex == row) {
                    return createIndex(row, column, (void*)bone);
                }
            }
        }

        return QModelIndex();
    }

    QModelIndex	parent(const QModelIndex & index) const
    {
        const Bone *bone = (const Bone*)index.internalPointer();

        if (!bone->parent()) {
            return QModelIndex();
        } else {
            const Bone *parent = bone->parent();

            if (!parent->parent()) {
                return createIndex(mRootBones.indexOf(parent), 0, (void*)parent);
            }

            int indexInParent = -1;
            foreach (const Bone *otherBone, mSkeleton->bones()) {
                if (otherBone->parent() == parent->parent()) {
                    indexInParent++;
                }
                if (otherBone == parent) {
                    return createIndex(indexInParent, 0, (void*)parent);
                }
            }
        }

        return QModelIndex();
    }

    int rowCount(const QModelIndex & parent) const
    {
        if (!parent.isValid()) {
            return mRootBones.size();
        } else {
            int count = 0;
            const Bone *parentBone = (const Bone*)parent.internalPointer();
            foreach (const Bone *bone, mSkeleton->bones()) {
                if (bone->parent() == parentBone)
                    count++;
            }
            return count;
        }
    }

    Qt::ItemFlags flags ( const QModelIndex & index ) const
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return "Name";
            }
        }

        return QVariant();
    }

private:
    const Skeleton *mSkeleton;
    QVector<const Bone*> mRootBones;
};

BoneHierarchy::BoneHierarchy(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BoneHierarchy)
{
    ui->setupUi(this);
}

BoneHierarchy::~BoneHierarchy()
{
    delete ui;
}

void BoneHierarchy::setSkeleton(const Skeleton *skeleton)
{
    if (!skeleton) {
        ui->treeView->setModel(NULL);
    } else {
        ui->treeView->setModel(new SkeletonModel(skeleton));
    }
}

#include "bonehierarchy.moc"

void BoneHierarchy::on_treeView_activated(QModelIndex index)
{
    if (!index.isValid()) {
        qDebug("Deactivating bone.");
        emit selectBone(-1);
    }

    const Bone *bone = (const Bone*)index.internalPointer();
    emit selectBone(bone->boneId());
    qDebug("Activating bone %d.", bone->boneId());
}
