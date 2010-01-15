#ifndef MESHDIALOG_H
#define MESHDIALOG_H

#include <QDialog>

namespace Ui {
    class MeshDialog;
}

namespace EvilTemple
{

    class GeometryMeshObject;

    class MeshDialog : public QDialog {
        Q_OBJECT
    public:
        MeshDialog(GeometryMeshObject *mesh, QWidget *parent = 0);
        ~MeshDialog();

    protected:
        void changeEvent(QEvent *e);

    private:

        Ui::MeshDialog *ui;
    };

}

#endif // MESHDIALOG_H
