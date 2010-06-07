
#include <QFileDialog>

#include "choosedirectorypage.h"
#include "ui_choosedirectorypage.h"
#include "basepathfinder.h"

ChooseDirectoryPage::ChooseDirectoryPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::ChooseDirectoryPage)
{
    ui->setupUi(this);

    registerField("installationPath", ui->path);

    setTitle("Choose Installation Directory");
    setSubTitle("Please choose the directory in which you installed Temple of Elemental Evil.\n"
                "The content of this directory will not be modified.");

    connect(ui->path, SIGNAL(textChanged(QString)), SLOT(updateStatus()));

    QDir initialDir = EvilTemple::BasepathFinder::find();

    ui->path->setText(QDir::toNativeSeparators(initialDir.path()));

    setCommitPage(true);

    updateStatus();
}

ChooseDirectoryPage::~ChooseDirectoryPage()
{
    delete ui;
}

void ChooseDirectoryPage::changeEvent(QEvent *e)
{
    QWizardPage::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ChooseDirectoryPage::on_choosePath_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Temple of Elemental Evil Installation Directory"),
                                                    ui->path->text(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if (dir.isNull())
        return;

    ui->path->setText(dir);

    updateStatus();
}

void ChooseDirectoryPage::updateStatus()
{
    QString path = ui->path->text();

    if (path.isEmpty()) {
        ui->statusText->setVisible(false);
        ui->statusIcon->setVisible(false);
        mComplete = false;
        return;
    }

    ui->statusIcon->setVisible(true);
    ui->statusText->setVisible(true);

    bool success = true;
    QString statusText = "The installation directory seems to be valid.";

    QDir dir(path);

    if (!dir.exists()) {
        success = false;
        statusText = tr("The chosen directory doesn't exist.").arg(path);
    } else if (!dir.isReadable()) {
        success = false;
        statusText = tr("You lack permission to read the directory.").arg(path);
    } else {
        // Check for ToEE1.dat, ToEE2.dat, ToEE3.dat, modules/ToEE.dat
        QStringList checkFor;
        checkFor << "ToEE1.dat" << "ToEE2.dat" << "ToEE3.dat" << QDir::toNativeSeparators("modules/ToEE.dat");
        QStringList missingFiles;
        foreach (const QString &filename, checkFor) {
            QFile file(dir.absoluteFilePath(filename));
            if (!file.exists()) {
                missingFiles.append(filename);
            }
        }

        if (!missingFiles.isEmpty()) {
            success = false;
            statusText = tr("The directory you selected is missing the following files:");
            foreach (const QString &missingFile, missingFiles) {
                statusText.append("\n");
                statusText.append(missingFile);
            }
        }
    }

    if (success) {
        ui->statusIcon->setPixmap(QPixmap(":/24-em-check.png"));
    } else {
        ui->statusIcon->setPixmap(QPixmap(":/24-em-cross.png"));
    }
    ui->statusText->setText(statusText);
    mComplete = success;
    emit completeChanged();
}

bool ChooseDirectoryPage::isComplete() const
{
    return mComplete;
}

QString ChooseDirectoryPage::chosenDirectory() const
{
    return ui->path->text();
}
