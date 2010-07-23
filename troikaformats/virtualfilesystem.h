#ifndef VIRTUALFILESYSTEM_H
#define VIRTUALFILESYSTEM_H

#include "troikaformatsglobal.h"

#include <QObject>
#include <QFile>
#include <QList>
#include <QDir>

namespace Troika {

/*!
    This interface is the base for all virtual file system handlers.
*/
class TROIKAFORMATS_EXPORT VirtualFileSystemHandler {
public:
    virtual QByteArray openFile(const QString &filename) = 0;
    virtual bool exists(const QString &filename) = 0;
    virtual QStringList listFiles(const QString &path, const QString &filter = "*") = 0;
    virtual QStringList listAllFiles(const QString &filenameFilter) = 0;
};

class TROIKAFORMATS_EXPORT VirtualFileSystem : public QObject
{
Q_OBJECT
public:
    typedef VirtualFileSystemHandler *Handler;

    explicit VirtualFileSystem(QObject *parent = 0);

    QByteArray openFile(const QString &filename);
    bool exists(const QString &filename);
    QStringList listFiles(const QString &path, const QString &filter = "*");
    QStringList listAllFiles(const QString &filenameFilter);

    void loadDefaultArchives(const QString &basepath);

signals:

public slots:
    void add(Handler handler);
    void remove(Handler handler);

private:
    QList<Handler> handler;

    Q_DISABLE_COPY(VirtualFileSystem)
};

}

#endif // VIRTUALFILESYSTEM_H
