
#include <QDesktopServices>
#include <QCoreApplication>

#include "common/paths.h"

using namespace EvilTemple;

static const QString DataDirName("EvilTemple");
QDir getOrCreateUserDataDir();
QDir getOrCreateGeneratedDataDir();
QDir getInstallationDir();

class PathsData {
public:
    QDir userDataDir;
    QDir generatedDataDir;
    QDir installationDir;
    QString userDataPath;
    QString generatedDataPath;
    QString installationPath;
};

Paths::Paths(bool disableAutodetection, QObject *parent) :
    QObject(parent), d(new PathsData)
{
    if (!disableAutodetection)
        autodetectPaths();
}

Paths::~Paths()
{
    delete d;
}

void Paths::autodetectPaths()
{
    QStringList arguments = QCoreApplication::instance()->arguments();

    bool userDataDirOverriden = false;
    bool generatedDataDirOverriden = false;
    bool installationDirOverriden = false;

    for (int i = 1; i + 1 < arguments.size(); ++i) {
        const QString &argument = arguments.at(i);

        if (argument == "-installationPath") {
            QString path = arguments.at(i+1);

            QDir dir(path);
            if (!dir.exists()) {
                qFatal("When the installation path is overriden on the command-line, the directory must exist: %s",
                       qPrintable(path));
            }

            printf("Forcing installation path to: %s", qPrintable(path));
            setInstallationDir(path);
            installationDirOverriden = true;

            i++; // Skip the next token
        } else if (argument == "-userDataPath") {
            QString path = arguments.at(i+1);

            QDir dir(path);
            if (!dir.exists()) {
                qFatal("When the user data path is overriden on the command-line, the directory must exist: %s",
                       qPrintable(path));
            }

            printf("Forcing user data path to: %s", qPrintable(path));
            setInstallationDir(path);
            installationDirOverriden = true;

            i++; // Skip the next token
        } else if (argument == "-generatedDataPath") {
            QString path = arguments.at(i+1);

            QDir dir(path);
            if (!dir.exists()) {
                qFatal("When the generated data path is overriden on the command-line, the directory must exist: %s",
                       qPrintable(path));
            }

            qDebug("Forcing user data path to: %s", qPrintable(path));
            setInstallationDir(path);
            installationDirOverriden = true;

            i++; // Skip the next token
        }
    }

    if (!userDataDirOverriden)
        setUserDataDir(getOrCreateUserDataDir());
    if (!generatedDataDirOverriden)
        setGeneratedDataDir(getOrCreateGeneratedDataDir());
    if (!installationDirOverriden)
        setInstallationDir(getInstallationDir());
}

const QDir &Paths::userDataDir() const
{
    return d->userDataDir;
}

const QDir &Paths::generatedDataDir() const
{
    return d->generatedDataDir;
}

const QDir &Paths::installationDir() const
{
    return d->installationDir;
}

const QString &Paths::userDataPath() const
{
    return d->userDataPath;
}

const QString &Paths::generatedDataPath() const
{
    return d->generatedDataPath;
}

const QString &Paths::installationPath() const
{
    return d->installationPath;
}

void Paths::setUserDataDir(const QDir &dir)
{
    d->userDataDir = dir;
    d->userDataPath = dir.absolutePath();
    if (!d->userDataPath.endsWith(QDir::separator()))
        d->userDataPath.append(QDir::separator());
    qDebug("User Data Path: %s", qPrintable(d->userDataPath));
}

void Paths::setGeneratedDataDir(const QDir &dir)
{
    d->generatedDataDir = dir;
    d->generatedDataPath = dir.absolutePath();
    if (!d->generatedDataPath.endsWith(QDir::separator()))
        d->generatedDataPath.append(QDir::separator());
    qDebug("Generated Data Path: %s", qPrintable(d->generatedDataPath));
}

void Paths::setInstallationDir(const QDir &dir)
{
    d->installationDir = dir;
    d->installationPath = dir.absolutePath();
    if (!d->installationPath.endsWith(QDir::separator()))
        d->installationPath.append(QDir::separator());
    qDebug("Installation Path: %s", qPrintable(d->installationPath));
}

static QDir getOrCreateUserDataDir()
{
    QDir dir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);

    if (!dir.exists(DataDirName)) {
        qDebug("Creating user data directory %s in %s",
                qPrintable(dir.absolutePath()),
                qPrintable(DataDirName));
        dir.mkdir(DataDirName);
    }

    if (!dir.cd(DataDirName)) {
       qDebug("Can't change into the user data directory %s in %s.",
              qPrintable(dir.absolutePath()),
              qPrintable(DataDirName));
    }

    return dir;
}

static QDir getOrCreateGeneratedDataDir()
{
    QDir dir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);

    if (!dir.exists(DataDirName)) {
        qDebug("Creating generated data directory %s in %s",
                qPrintable(dir.absolutePath()),
                qPrintable(DataDirName));
        dir.mkdir(DataDirName);
    }

    if (!dir.cd(DataDirName)) {
       qDebug("Can't change into the generated data directory %s in %s.",
              qPrintable(dir.absolutePath()),
              qPrintable(DataDirName));
    }

    return dir;
}

static QDir getInstallationDir()
{
    QCoreApplication *app = QCoreApplication::instance();

    QDir dir = app->applicationDirPath();
    dir.cdUp();

    return dir;
}
