#include <QSet>

#include "virtualfilesystem.h"
#include "troikaarchive.h"

namespace Troika {

VirtualFileSystem::VirtualFileSystem(QObject *parent) :
    QObject(parent)
{
}

void VirtualFileSystem::add(Handler handler) {
    this->handler.append(handler);
}

void VirtualFileSystem::remove(Handler handler) {
    this->handler.removeAll(handler);
}

QByteArray VirtualFileSystem::openFile(const QString &filename) {
    foreach (Handler handler, this->handler) {
        QByteArray result = handler->openFile(filename);
        if (!result.isNull()) {
            return result;
        }
    }

    return QByteArray((char*)NULL, 0);
}

bool VirtualFileSystem::exists(const QString &filename) {
    foreach (Handler handler, this->handler) {
        if (handler->exists(filename))
            return true;
    }

    return false;
}

QStringList VirtualFileSystem::listFiles(const QString &path, const QString &filter) {
    QStringList result;

    foreach (Handler handler, this->handler)  {
        result.append(handler->listFiles(path, filter));
    }

    return result.toSet().toList();
}

QStringList VirtualFileSystem::listAllFiles(const QString &filter)
{
    QStringList result;

    foreach (Handler handler, this->handler)  {
        result.append(handler->listAllFiles(filter));
    }

    return result.toSet().toList();
}

void VirtualFileSystem::loadDefaultArchives(const QString &basePath)
{
    // Add base archives
    for (int i = 1; i <= 4; ++i) {
        QString archivePath = QString("%1ToEE%2.dat").arg(basePath).arg(i);

        if (QFile::exists(archivePath)) {
            qDebug("Adding archive %s", qPrintable(archivePath));
            add(new Troika::TroikaArchive(archivePath, this));
        }
    }

    QString modulePath = QString("%1modules%2ToEE.dat").arg(basePath).arg(QDir::separator());
    qDebug("Adding archive %s", qPrintable(modulePath));
    add(new Troika::TroikaArchive(modulePath, this));
}

}
