#include <QSet>

#include "virtualfilesystem.h"

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

}
